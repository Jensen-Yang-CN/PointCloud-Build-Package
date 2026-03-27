#ifndef IMUWORKER_H
#define IMUWORKER_H

#include <QObject>
#include "BaseWorker.h"

class ImuWorker : public BaseWorker
{
    Q_OBJECT
public:
    explicit ImuWorker(QObject *parent = nullptr);
    ~ImuWorker();


signals:

public slots:
    void processPacket(QByteArray& payload);



};

#endif // IMUWORKER_H
