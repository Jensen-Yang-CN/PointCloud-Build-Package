#include "UdpDispatcher.h"
#include "common/types.h"   // PacketPtr
#include <QThread>     // ⭐ 必须加
#include <QPointer>
#include <QMetaObject>

UdpDispatcher::UdpDispatcher(QObject *parent)
{
    (void)parent;
}

void UdpDispatcher::dispatch(const QHostAddress &ip,
                             quint16 port,
                             const QByteArray &payload,
                             double timestamp)
{
    // ===== 1. 查表=====
    RouteKey key{ip, port};

    auto it = routeTable.constFind(key);
    if (it == routeTable.constEnd()) {
        return;
    }

    const Rule &rule = it.value();
    if (!rule.receiver) return;

    // ===== 2. 构造零拷贝数据 =====
    PacketPtr pkt = PacketPtr::create(payload);

    // ===== 3. 线程判断 =====
    QThread* targetThread = rule.receiver->thread();
    QThread* currentThread = QThread::currentThread();

    // ===== 4. 同线程（最快路径）=====
    if (targetThread == currentThread) {
        rule.callback(*pkt, timestamp);
        return;
    }

    // ===== 5. 跨线程（安全 + 高性能）=====

    // 防止对象已被销毁
    QPointer<QObject> safeReceiver = rule.receiver;

    // 拷贝轻量对象（避免捕获整个 rule）
    auto cb = rule.callback;

    QMetaObject::invokeMethod(rule.receiver,
        [safeReceiver, cb, pkt, timestamp]() {

            // ⭐ 防止 receiver 已析构
            if (!safeReceiver) return;

            // ⭐ 真正执行
            cb(*pkt, timestamp);

        },
        Qt::QueuedConnection
    );
}
