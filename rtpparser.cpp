#include "rtpparser.h"
#include <QtEndian>
#include <QDebug>

void RtpParser::inputPacket(const QByteArray &packet, double timestamp)
{
    //(void)timestamp;
    //qDebug() << "Yes rule for:RtpParser::inputPacket" << m_ip<<":ip:"<<m_ip<<":port:"<<m_port;

    if (packet.size() < 12) return;

    const uchar* data = (const uchar*)packet.data();

    quint8 cc = data[0] & 0x0F;
    int headerSize = 12 + cc * 4;

    if (packet.size() <= headerSize) return;

    QByteArray payload = packet.mid(headerSize);

    // ===== H265 =====
    quint8 nalType = (payload[0] >> 1) & 0x3F;

    // ✅ 单NAL（直接输出）
    if (nalType >= 0 && nalType <= 40) {
        emit rtpPayloadReady(payload,timestamp);
        return;
    }

    // =========================
    // 🔥 FU 分片（重点）
    // =========================
    if (nalType == 49) {

        quint8 fuHeader = payload[2];

        bool start = fuHeader & 0x80;
        bool end   = fuHeader & 0x40;

        quint8 originalNalType = fuHeader & 0x3F;

        if (start) {
            fuBuffer.clear();

            // 重建 NAL Header（2字节）
            quint8 header0 = (payload[0] & 0x81) | (originalNalType << 1);
            quint8 header1 = payload[1];

            fuBuffer.append(header0);
            fuBuffer.append(header1);

            fuBuffer.append(payload.mid(3));

            fuStarted = true;
        }
        else if (fuStarted) {
            fuBuffer.append(payload.mid(3));
        }

        if (end && fuStarted) {
            emit rtpPayloadReady(fuBuffer,timestamp);
            fuBuffer.clear();
            fuStarted = false;
        }

        return;
    }

    // =========================
    // STAP-A（简单支持）
    // =========================
    if (nalType == 48) {
        int offset = 2;

        while (offset + 2 < payload.size()) {
            quint16 size = qFromBigEndian<quint16>((uchar*)payload.data() + offset);
            offset += 2;

            if (offset + size > payload.size()) break;

            QByteArray nal = payload.mid(offset, size);
            emit rtpPayloadReady(nal,timestamp);

            offset += size;
        }
    }
}
