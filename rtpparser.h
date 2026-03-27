#pragma once
#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QHostAddress>

struct FrameBuffer {
    QByteArray data;
    quint32 timestamp;
};

class RtpParser : public QObject {
    Q_OBJECT
public:
    explicit RtpParser(const QHostAddress &ip, quint16 port, QObject *parent = nullptr)
            : QObject(parent), m_ip(ip), m_port(port) {}
public slots:
    void inputPacket(const QByteArray &data, double timestamp);

signals:
    void rtpPayloadReady(const QByteArray &nal,double timestamp);

private:
    QByteArray fuBuffer;
    bool fuStarted = false;
    QString m_name;
    QHostAddress m_ip;
    quint16 m_port;
};
