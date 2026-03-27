#ifndef RsLidarWidget_H
#define RsLidarWidget_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include "common/pointxyz.h" // 引用你的 PointXYZI

class RsLidarWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit RsLidarWidget(QWidget *parent = nullptr);
    ~RsLidarWidget();

public slots:
    // 接收来自 RsLidarWorker 的信号
    void updateCloud(const QVector<PointXYZI> &cloud);

protected:
    // OpenGL 核心生命周期函数
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    // 鼠标控制（用于旋转和缩放视角）
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLBuffer m_vbo;

    QVector<PointXYZI> m_currentCloud;
    int m_pointCount = 0;

    // 视角控制变量
    float m_rotationX = -45.0f;
    float m_rotationY = 45.0f;
    float m_zOffset = -50.0f;
    QPoint m_lastMousePos;
    QVector<PointXYZI> m_allPoints; // 全局存储
    const int MAX_POINTS = 90000;  // 限制最大显示点数，防止显卡卡死
};

#endif // RsLidarWidget_H
