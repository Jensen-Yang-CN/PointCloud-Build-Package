#ifndef POINTXYZ_H
#define POINTXYZ_H

#include <QtGlobal>

struct PointXYZI {
    float x;
    float y;
    float z;
    float intensity;
};

template<typename T>
struct TimedFrame {
    double timestamp;
    T data;
};

#endif // POINTXYZ_H
