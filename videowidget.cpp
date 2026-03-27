#include "videowidget.h"
#include <QPainter>
#include <QDebug>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{

}

void VideoWidget::updateFrame(const QImage &img)
{
    //qDebug() << "VideoWidget 收到新画面，分辨率:" << img.width() << "x" << img.height();
//    //QMutexLocker locker(&mutex);
//    frame = img.copy();
//    update(); // 触发 paintEvent
    if(!isFirst){
        qDebug() << "VideoWidget::updateFrame:"<<img ;
        isFirst = true;
    }
    frame = img;
    this->update(); // 这里是在主线程触发重绘，是安全的
}

void VideoWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::black);

    mutex.lock();
    if(!frame.isNull())
    {
        QImage scaled = frame.scaled(size(), Qt::KeepAspectRatio);
        int x = (width()-scaled.width())/2;
        int y = (height()-scaled.height())/2;
        p.drawImage(x, y, scaled);
    }
    mutex.unlock();
}

