#include "PcapngReader.h"
#include <pcap.h>
#include <QDebug>
#include <QtEndian>
#include <QElapsedTimer>
#include <QThread>

PcapngReader::PcapngReader(QObject *parent) : QObject(parent) {}

bool PcapngReader::open(const QString &file) {
    qDebug() << "PcapngReader::open -> 开始流式读取";
    char errbuf[PCAP_ERRBUF_SIZE];

    // 重置停止标志（如果类定义中已有该原子变量）
    m_stopRequested = false;

    QByteArray ba = file.toLocal8Bit();
    pcap_t* handle = pcap_open_offline(ba.data(), errbuf);
    if (!handle) {
        qDebug() << "open failed:" << errbuf;
        return false;
    }

    struct pcap_pkthdr* header;
    const u_char* data;

    // --- 控速器变量 ---
    QElapsedTimer wallClock;      // 本地计时器，用于对齐现实时间
    double firstPacketTime = -1.0; // 记录文件中第一包的时间戳基准

    int res;
    // 使用 pcap_next_ex 逐包读取，不预加载
    while (!m_stopRequested && (res = pcap_next_ex(handle, &header, &data)) >= 0) {
        if (res == 0) continue; // 超时，继续

        // --- 1. 时间轴同步逻辑 ---
        double currentPacketTime = header->ts.tv_sec + (header->ts.tv_usec / 1000000.0);

        if (firstPacketTime < 0) {
            // 这是第一包，初始化基准
            firstPacketTime = currentPacketTime;
            wallClock.start();
        } else {
            // 计算当前包相对于第一包的理论延迟 (毫秒)
            double targetElapsedMs = (currentPacketTime - firstPacketTime) * 1000.0;
            qint64 actualElapsedMs = wallClock.elapsed();

            // 如果还没到理论发射时间，进行微秒级睡眠
            if (targetElapsedMs > actualElapsedMs) {
                QThread::msleep(static_cast<unsigned long>(targetElapsedMs - actualElapsedMs));
            }
        }

        // --- 2. 协议解析 (保持你的高效指针偏移法) ---
        // 判定最小长度：以太网(14) + IPv4最小(20) + UDP(8) = 42
        if (header->caplen < 42) continue;

        const u_char* ip = data + 14;
        if (((ip[0] >> 4) & 0x0F) != 4) continue; // 仅处理 IPv4

        quint8 ihl = (ip[0] & 0x0F) * 4;
        if (ihl < 20 || (14 + ihl + 8) > header->caplen) continue;

        const u_char* udp = ip + ihl;

        // 提取端口与IP
        quint16 srcPort = qFromBigEndian<quint16>(*(quint16*)(udp));
        quint32 ipAddr = *reinterpret_cast<const quint32*>(ip + 12);
        QHostAddress srcIp(qFromBigEndian(ipAddr));

        // 3. 提取 Payload 并发射
        // 注意：header->len 是原始长度，header->caplen 是实际抓到的长度
        int payloadLen = header->caplen - (14 + ihl + 8);
        if (payloadLen > 0) {
            // 完全保持你原始的信号接口不变
            emit udpPacket(srcIp, srcPort, QByteArray((const char*)(udp + 8), payloadLen), currentPacketTime);
        }
    }

    if (res == -1) qDebug() << "读取出错:" << pcap_geterr(handle);

    pcap_close(handle);
    qDebug() << "PcapngReader: 读取结束";
    return true;
}
