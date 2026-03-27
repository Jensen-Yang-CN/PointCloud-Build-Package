#include "Ls400Parser.h"
#include <QtEndian>
#include <cmath>

Ls400Parser::Ls400Parser(const QHostAddress &ip, quint16 port, QObject *parent)
    : QObject(parent), m_ip(ip), m_port(port) {
    initTables();
}

void Ls400Parser::initTables()
{
    // 初始化水平和垂直角度表
    for(int i=0;i<1024;++i) {
        horizTable[i] = i * (360.0f/1024.0f) * M_PI/180.0f; // 弧度
        vertTable[i]  = 0.0f; // 按雷达官方角度填入
    }
}



void Ls400Parser::inputPacket(const QByteArray& packet, double timestamp)
{
    if(!isFirst){
        qDebug()<<"Ls400Parser已收到分流的数据包，可解析LS400线雷达数据";
        isFirst = true;
    }

    if(packet.size() < 1000) return;

    const uint8_t* data = reinterpret_cast<const uint8_t*>(packet.data());

    uint16_t seq = qFromLittleEndian<uint16_t>(*(uint16_t*)data);

    if(seq != lastSeq + 1 && !currentFrame.isEmpty()) {
        emit frameReady({frameStartTs, currentFrame});
        currentFrame.clear();
        frameStartTs = timestamp;
    }

    lastSeq = seq;

    int offset = 16;
    int count = (packet.size() - offset)/4;

    for(int i=0; i<count; ++i)
    {
        int idx = offset + i*4;
        uint16_t dist_raw = qFromLittleEndian<uint16_t>(*(uint16_t*)(data+idx));
        uint8_t intensity = *(uint8_t*)(data+idx+2);
        uint8_t angleIdx = *(uint8_t*)(data+idx+3);

        float dist = dist_raw * 0.001f;
        float theta = horizTable[angleIdx];
        float phi   = vertTable[angleIdx];

        float x = dist * cos(phi) * cos(theta);
        float y = dist * cos(phi) * sin(theta);
        float z = dist * sin(phi);

        currentFrame.push_back({x, y, z, (float)intensity});
    }
}
