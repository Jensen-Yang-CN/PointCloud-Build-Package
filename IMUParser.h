#ifndef IMUPARSER_H
#define IMUPARSER_H

#include <QObject>
#include <QHostAddress>

class IMUParser : public QObject
{
    Q_OBJECT
public:
    explicit IMUParser(const QHostAddress &ip, quint16 port,QObject *parent = nullptr)
        :QObject(parent),m_ip(ip),m_port(port){}

signals:
    void rtpPayloadReady(const QByteArray &nal,double timestamp);

public slots:
    void inputPacket(const QByteArray &packet, double timestamp);
private:
    QHostAddress m_ip;
    quint16 m_port;
    bool isFirst = false;
};

#endif // IMUPARSER_H
