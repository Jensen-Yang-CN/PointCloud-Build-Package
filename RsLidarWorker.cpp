#include "RsLidarWorker.h"
#include <QDebug>
#include <QtEndian>

RsLidarWorker::RsLidarWorker(QObject *parent) : BaseWorker(parent)
{
    // 跨线程传输自定义结构体必须注册元类型
    qRegisterMetaType<LidarFrame>("LidarFrame");
    qRegisterMetaType<QVector<PointXYZI>>("QVector<PointXYZI>");
}

RsLidarWorker::~RsLidarWorker() {}

void RsLidarWorker::processPacket(QByteArray payload)
{
    // BaseWorker 要求的接口，如果不需要直接处理原始包，可以留空
    // 所有的解析工作已经在 Parser 中由 inputPacket 完成了
    (void)payload;
}

void RsLidarWorker::handleFrame(const LidarFrame &frame)
{
    if (frame.data.isEmpty()) return;

    // --- 这里可以添加后续处理逻辑 ---
    // 例如：1. 过滤掉距离过近的点（避开雷达支架自身）
    //      2. 坐标纠偏
    //      3. 简单的统计打印
    // qDebug() << "Worker 处理帧，点数:" << frame.data.size() << " 时间戳:" << frame.timestamp;

    // 直接将点云数据投递给 UI 层的 LidarWidget
    //qDebug()<<"RsLidarWorker::handleFrame --> emit cloudReady(frame.data);";
    emit cloudReady(frame.data);

    // 如果需要通知 UI 更新状态（比如当前频率），也可以在这里 emit 其他信号
}
