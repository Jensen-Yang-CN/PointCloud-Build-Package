#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QImage>
#include <QMutex>

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);

signals:

public slots:
    void updateFrame(const QImage& img);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QImage frame;
    QMutex mutex;
    bool isFirst = false;
};

#endif // VIDEOWIDGET_H
