#ifndef RSLIDARPARSER_H
#define RSLIDARPARSER_H

#include <QObject>
#include <QVector>
#include "common/pointxyz.h"
#include "common/types.h"
#include <QHostAddress>


class RsLidarParser : public QObject
{
    Q_OBJECT
public:
    explicit RsLidarParser(const QHostAddress &ip, quint16 port, QObject *parent = nullptr);
public slots:
    void inputPacket(const QByteArray& packet, double timestamp);

signals:
    void frameReady(TimedFrame<QVector<PointXYZI>> frame);

private:
    void initVerticalTable(); // 初始化雷达垂直角度

private:
    QVector<PointXYZI> currentFrame;
    float lastAzimuth = 0;
    double frameStartTs = 0;
    float verticalTable[32]; // 雷达垂直角度表
    QHostAddress m_ip;
    quint16 m_port;
    bool isFirst = false;



};
#endif // RSLIDARPARSER_H


