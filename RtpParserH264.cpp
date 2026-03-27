#include "RtpParserH264.h"

void RtpParserH264::inputPacket(const QByteArray &data, double timestamp) {
    if (data.size() < 13) return;

    // 跳过 12 字节 RTP Header
    const unsigned char* rtpPayload = reinterpret_cast<const unsigned char*>(data.data() + 12);
    int rtpPayloadSize = data.size() - 12;

    unsigned char indicator = rtpPayload[0];
    unsigned char nalType = indicator & 0x1F;
    const char startCode[] = {0x00, 0x00, 0x00, 0x01};

    // --- 情况 A: FU-A 分片包 (Type 28) ---
    if (nalType == 28) {
        unsigned char fuHeader = rtpPayload[1];
        bool startBit = (fuHeader & 0x80) != 0;
        bool endBit   = (fuHeader & 0x40) != 0;
        unsigned char trueNalType = fuHeader & 0x1F;

        if (startBit) {
            m_fuBuffer.clear();

            // 【关键逻辑】如果是 20080 的关键帧 (Type 5)，且我们手头有 20082 的参数
            if (trueNalType == 5 && !m_extraConfig.isEmpty()) {
                m_fuBuffer.append(m_extraConfig); // 先塞钥匙
            }

            m_fuBuffer.append(startCode, 4);
            unsigned char reconstructedHeader = (indicator & 0xE0) | trueNalType;
            m_fuBuffer.append(static_cast<char>(reconstructedHeader));
            m_fuBuffer.append(reinterpret_cast<const char*>(rtpPayload + 2), rtpPayloadSize - 2);
            m_isWaitingForEnd = true;
        }
        else if (m_isWaitingForEnd) {
            m_fuBuffer.append(reinterpret_cast<const char*>(rtpPayload + 2), rtpPayloadSize - 2);
            if (endBit) {
                emit rtpPayloadReady(m_fuBuffer, timestamp);
                m_isWaitingForEnd = false;
            }
        }
    }
    // --- 情况 B: 单个 NAL 包 (Type 1-23, SPS/PPS 通常落在这里) ---
    else if (nalType >= 1 && nalType <= 23) {
        QByteArray singleUnit;

        // 如果是普通 P 帧 (Type 1) 且有参数备份，也拼上去提高解码成功率
        if (nalType == 1 && !m_extraConfig.isEmpty()) {
            singleUnit.append(m_extraConfig);
        }

        singleUnit.append(startCode, 4);
        singleUnit.append(reinterpret_cast<const char*>(rtpPayload), rtpPayloadSize);

        emit rtpPayloadReady(singleUnit, timestamp);
        m_isWaitingForEnd = false;
    }
    // --- 情况 C: STAP-A 组合包 (Type 24) ---
    else if (nalType == 24) {
        int offset = 1;
        while (offset < rtpPayloadSize - 2) {
            unsigned short subSize = (static_cast<unsigned char>(rtpPayload[offset]) << 8) |
                                      static_cast<unsigned char>(rtpPayload[offset + 1]);
            offset += 2;
            if (offset + subSize <= rtpPayloadSize) {
                QByteArray subUnit;
                subUnit.append(startCode, 4);
                subUnit.append(reinterpret_cast<const char*>(rtpPayload + offset), subSize);
                emit rtpPayloadReady(subUnit, timestamp);
                offset += subSize;
            } else break;
        }
    }
}
