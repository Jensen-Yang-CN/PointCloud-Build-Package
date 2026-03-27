#ifndef WORKERTHREADWRAPPER_H
#define WORKERTHREADWRAPPER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include "BaseWorker.h"

class WorkerThreadWrapper : public QObject
{
    Q_OBJECT
public:
    explicit WorkerThreadWrapper(BaseWorker* worker, QObject* parent = nullptr)
        : QObject(parent), m_worker(worker)
    {
        m_thread = new QThread(this);
        m_worker->moveToThread(m_thread);

        connect(m_thread, &QThread::started, this, &WorkerThreadWrapper::onStart);
        connect(m_worker, &BaseWorker::finished, m_thread, &QThread::quit);
        connect(m_worker, &BaseWorker::finished, m_worker, &QObject::deleteLater);
        connect(m_worker, &BaseWorker::errorOccurred, this, &WorkerThreadWrapper::onError);
        connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);

        m_thread->start();
    }

    ~WorkerThreadWrapper()
    {
        if (m_thread->isRunning()) {
            m_thread->quit();
            m_thread->wait();
        }
    }

    BaseWorker* worker() const { return m_worker; }

private slots:
    void onStart() {
        // 可以做 Worker 初始化，或者首次 processPacket
    }

    void onError(const QString &msg) {
        qWarning() << "WorkerThreadWrapper caught error:" << msg;
    }

private:
    QThread* m_thread;
    BaseWorker* m_worker;
};
#endif // WORKERTHREADWRAPPER_H
