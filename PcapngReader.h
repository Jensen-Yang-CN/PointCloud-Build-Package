#pragma once
#include <QObject>
#include <QHostAddress>

class PcapngReader : public QObject {
    Q_OBJECT
public:
    explicit PcapngReader(QObject *parent = nullptr);

    bool open(const QString &file);

signals:
    void udpPacket(QHostAddress ip, quint16 port, QByteArray payload,double timestamp);
private:
    std::atomic<bool> m_stopRequested{false}; // 用于安全停止
};
