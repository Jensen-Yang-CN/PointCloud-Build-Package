#include "ImuWorker.h"
#include <QDataStream>
#include <QDebug>

ImuWorker::ImuWorker(QObject *parent) : BaseWorker(parent)
{

}

ImuWorker::~ImuWorker()
{

}

void ImuWorker::processPacket(QByteArray &payload)
{
    // ⚠️ 根据你设备协议改
    if(payload.size() < 24) return;
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::LittleEndian);
    float ax, ay, az;
    float gx, gy, gz;

    ds >> ax >> ay >> az;
    ds >> gx >> gy >> gz;

    qDebug()<<"IMU:"
            <<"acc:"<<ax<<ay<<az
            <<"gyro:"<<gx<<gy<<gz;

}
