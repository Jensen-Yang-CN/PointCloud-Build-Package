#ifndef RTPPARSERH264_H
#define RTPPARSERH264_H

#include <QObject>
#include <QByteArray>
#include <QHostAddress>

class RtpParserH264 : public QObject
{
    Q_OBJECT
public:
    explicit RtpParserH264(const QHostAddress &ip, quint16 port, QObject *parent = nullptr)
        : QObject(parent), m_ip(ip), m_port(port) {
        m_fuBuffer.reserve(1024 * 512); // 预留512KB
    }

    void setParserName(const QString &name) { parserName = name; }

signals:
    // 发送给 VideoWorker 的信号
    void rtpPayloadReady(const QByteArray &nal, double timestamp);

public slots:
    // 由 UdpDispatcher 调用
    void inputPacket(const QByteArray &data, double timestamp);

    // 【新增槽函数】接收来自 20082 端口解析器的 SPS/PPS 数据
    void onExtraConfigReceived(const QByteArray &config, double timestamp) {
        Q_UNUSED(timestamp);
        // 缓存这串“钥匙”，后续拼接到 20080 的 I 帧前
        // config 已经是带了起始码的完整 NAL 单元
        m_extraConfig = config;
    }

private:
    QHostAddress m_ip;
    quint16 m_port;
    QString parserName;
    QByteArray m_fuBuffer;      // 分片组包缓存
    QByteArray m_extraConfig;   // 缓存 20082 的参数包
    bool m_isWaitingForEnd = false;
};

#endif
