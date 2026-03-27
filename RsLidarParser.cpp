#include "RsLidarParser.h"
#include "LidarPacketStruct.h"
#include "common/pointxyz.h"
#include <QtEndian>
#include <cmath>
#include <QDebug>
#include <QMetaType>



// 在程序启动时或 Parser 构造时执行

RsLidarParser::RsLidarParser(const QHostAddress &ip, quint16 port, QObject *parent)
    : QObject(parent), m_ip(ip), m_port(port) {
    qRegisterMetaType<TimedFrame<QVector<PointXYZI>>>("TimedFrame<QVector<PointXYZI>>");
    initVerticalTable();
}


void RsLidarParser::initVerticalTable()
{
    // 假设 Helios 32 线雷达标准垂直角度（单位弧度）
    float table[32] = { -30.67, -9.33, -29.33, -8.0, -28.0, -6.66, -26.66, -5.33,
                        -25.33, -4.0, -24.0, -2.66, -22.66, -1.33, -21.33, 0.0,
                        -20.0, 1.33, -18.66, 2.66, -17.33, 4.0, -16.0, 5.33,
                        -14.66, 6.66, -13.33, 8.0, -12.0, 9.33, -10.66, 10.67};
    for(int i=0;i<32;++i) verticalTable[i] = table[i];
}

void RsLidarParser::inputPacket(const QByteArray& packet, double timestamp)
{
    // 1. 动态对齐同步头 FF EE
    int offset = packet.indexOf("\xff\xee");
    if (offset == -1 || packet.size() < offset + 1200) return;

    const char* data = packet.data() + offset;
    const LidarPacket* lp = reinterpret_cast<const LidarPacket*>(data);

    for(int b = 0; b < 12; ++b) {
        const LidarBlock& block = lp->blocks[b];

        // 校验当前 Block 的同步头是否对齐
        if (qFromLittleEndian<uint16_t>(block.header) != 0xEEFF) continue;

        // 2. 解析方位角 (Azimuth)
        // 直接读取，不再使用下标
        uint16_t azi_raw = qFromLittleEndian<uint16_t>(block.azimuth);
        float azimuth = azi_raw * 0.01f;

        // 容错处理：雷达有时会传出异常大的原始值
        if (azimuth >= 360.0f) azimuth = fmod(azimuth, 360.0f);

        // 3. 遍历通道填充点云
        for(int ch = 0; ch < 32; ++ch) {
            uint16_t dist_raw = qFromLittleEndian<uint16_t>(block.units[ch].distance);
            float dist = dist_raw * 0.002f;

            if (dist > 0.4f && dist < 150.0f) {
                float vAngle = verticalTable[ch];
                float azi_rad = azimuth * M_PI / 180.0f;

                // 激光雷达坐标系转换
                float x = dist * cos(vAngle) * cos(azi_rad);
                float y = dist * cos(vAngle) * sin(azi_rad);
                float z = dist * sin(vAngle);

                currentFrame.push_back({x, y, z, (float)block.units[ch].intensity});
            }
        }

        // 4. 【核心成帧逻辑改进】
        // 针对你之前“点数太少”且“频繁跳变”的问题：
        // 判定条件：角度回零 且 这一帧至少攒够了 10,000 个点
        if (azimuth < lastAzimuth && (lastAzimuth - azimuth) > 200) {
            if (currentFrame.size() > 10000) {
                //qDebug() << ">>> 成功合成完整帧 | 点数:" << currentFrame.size() << " | 耗时(ms):" << (timestamp - frameStartTs);
                emit frameReady({frameStartTs, currentFrame});
                currentFrame.clear();
                frameStartTs = timestamp;
            } else {
                // 如果点数不够，说明可能是噪声跳变，不清理缓存，继续积累
                // qDebug() << "跳变但点数不足，继续积累...";
            }
        }
        lastAzimuth = azimuth;
    }
}
