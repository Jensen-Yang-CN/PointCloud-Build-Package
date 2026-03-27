#ifndef RSLIDARWORKER_H
#define RSLIDARWORKER_H

#include "BaseWorker.h"
#include "common/pointxyz.h" // 引用你定义的结构体
#include <QVector>

// 定义类型别名方便使用
typedef TimedFrame<QVector<PointXYZI>> LidarFrame;

class RsLidarWorker : public BaseWorker
{
    Q_OBJECT
public:
    explicit RsLidarWorker(QObject *parent = nullptr);
    ~RsLidarWorker();

signals:
    // 发送给 LidarWidget 进行可视化的信号
    void cloudReady(const QVector<PointXYZI> &cloud);

public slots:
    // 实现 BaseWorker 的纯虚接口
    void processPacket(QByteArray payload) override;

    // 核心：接收来自 Parser 的流式点云帧
    // 参数匹配 emit frameReady({frameStartTs, currentFrame})
    void handleFrame(const LidarFrame &frame);

private:
    // 如果需要对点云进行预处理（如镜像、旋转），可以在这里定义私有方法
};

#endif // RSLIDARWORKER_H
