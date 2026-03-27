#ifndef STREAMRULE_H
#define STREAMRULE_H

#include <QHostAddress>
#include "rtpparser.h"
#include "VideoWorker.h"
#include <QLabel>
enum class StreamType
{
    H265_VIDEO,
    H264_VIDEO,
    RS_LIDAR,
    LS_LIDAR,
    IMU_STATUS,
    UNKNOWN
};

struct StreamThreadRule
{
    QObject* receiver;
    const char* slot; // QMetaObject::invokeMethod 用
};


#endif // STREAMRULE_H
