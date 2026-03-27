#ifndef LS400PARSER_H
#define LS400PARSER_H

#include <QObject>
#include <QVector>
#include "common/pointxyz.h"
#include "common/types.h"
#include <QHostAddress>

class Ls400Parser : public QObject
{
    Q_OBJECT
public:
    explicit Ls400Parser(const QHostAddress &ip, quint16 port, QObject *parent = nullptr);

public slots:
    void inputPacket(const QByteArray& packet, double timestamp);

signals:
    void frameReady(TimedFrame<QVector<PointXYZI>> frame);

private:
    void initTables();
private:
    QVector<PointXYZI> currentFrame;
    uint16_t lastSeq = 0;
    double frameStartTs = 0;
    float horizTable[1024];  // 水平角度表
    float vertTable[1024];   // 垂直角度表
    QHostAddress m_ip;
    quint16 m_port;
    bool isFirst = false;
};

#endif // LS400PARSER_H
