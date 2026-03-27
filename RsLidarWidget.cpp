#include "RsLidarWidget.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMatrix4x4>

RsLidarWidget::RsLidarWidget(QWidget *parent) : QOpenGLWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
}

RsLidarWidget::~RsLidarWidget() {
    makeCurrent();
    m_vbo.destroy();
    delete m_program;
    doneCurrent();
}

void RsLidarWidget::updateCloud(const QVector<PointXYZI> &cloud) {
    m_currentCloud = cloud;
    m_pointCount = cloud.size();
    // 1. 追加新点
    m_allPoints.append(cloud);

    // 2. 如果点数过多，删除最老的数据（维持一个滑动的空间窗口）
    if (m_allPoints.size() > MAX_POINTS) {
            int overflow = m_allPoints.size() - MAX_POINTS;
            m_allPoints.remove(0, overflow); // 剔除最老的一帧
        }

    // 3. 提交更新 ---> 触发重绘，paintGL 将被调用
    update();
//    const PointXYZI &p1 = cloud.first();
//    const PointXYZI &p2 = cloud.last();
//    qDebug() << QString("收到点云 | 点数: %1 | 首点: (%2, %3, %4) | 末点: (%5, %6, %7)")
//                .arg(cloud.size()).arg(p1.x).arg(p1.y).arg(p1.z).arg(p2.x).arg(p2.y).arg(p2.z);




}

void RsLidarWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 背景设为黑色
    glEnable(GL_DEPTH_TEST);              // 开启深度测试
    //::glPointSize(2.0f);                   // 让点大一点，方便观察

    m_program = new QOpenGLShaderProgram(this);
    // 顶点着色器：应用投影、视图、模型矩阵
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
        "attribute vec3 posAttr;\n"
        "attribute float intensityAttr;\n"
        "varying vec3 vColor;\n"
        "uniform mat4 matrix;\n"
        "void main() {\n"
        "   vColor = vec3(intensityAttr / 255.0, 1.0 - intensityAttr / 255.0, 0.5);\n" // 简单伪彩
        "   gl_Position = matrix * vec4(posAttr, 1.0);\n"
        "}\n");

    // 片元着色器：输出颜色
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
        "varying vec3 vColor;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(vColor, 1.0);\n"
        "}\n");
    m_program->link();

    m_vbo.create();
    m_vbo.bind();
    // 预留足够大的空间，防止频繁重新分配。RS-32 一帧约 32000 点
    m_vbo.allocate(100000 * sizeof(PointXYZI));
    m_vbo.release();
}

void RsLidarWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_allPoints.isEmpty()) return;

    m_program->bind();
    m_vbo.bind();

    // --- 关键：将当前点云数据写入 GPU ---
    // 使用 write 而不是 allocate，效率更高
    m_vbo.write(0, m_allPoints.constData(), m_allPoints.size() * sizeof(PointXYZI));

    // --- 设置矩阵 ---
//    QMatrix4x4 matrix;
//    matrix.perspective(45.0f, width() / (float)height(), 0.1f, 1000.0f);
//    matrix.translate(0, 0, m_zOffset); // 使用你的 m_zOffset (-50.0)
//    matrix.rotate(m_rotationX, 1, 0, 0);
//    matrix.rotate(m_rotationY, 0, 1, 0);
//    m_program->setUniformValue("matrix", matrix);


    QMatrix4x4 matrix;
    matrix.perspective(45.0f, width() / (float)height(), 0.1f, 1000.0f);
    matrix.translate(0, -2.0f, -60.0f); // 向上抬升相机 2米，后退 60米
    matrix.rotate(-60, 1, 0, 0);       // 俯视 60 度
    matrix.rotate(m_rotationY, 0, 0, 1); // 绕 Z 轴旋转查看四周
    m_program->setUniformValue("matrix", matrix);


    // --- 绑定顶点属性 ---
    // 对应 PointXYZI 结构体：x, y, z (float * 3)
    int posLoc = m_program->attributeLocation("posAttr");
    m_program->enableAttributeArray(posLoc);
    m_program->setAttributeBuffer(posLoc, GL_FLOAT, offsetof(PointXYZI, x), 3, sizeof(PointXYZI));

    // 对应 intensity (float)
    int intensityLoc = m_program->attributeLocation("intensityAttr");
    m_program->enableAttributeArray(intensityLoc);
    m_program->setAttributeBuffer(intensityLoc, GL_FLOAT, offsetof(PointXYZI, intensity), 1, sizeof(PointXYZI));

    // --- 执行绘制 ---
    glDrawArrays(GL_POINTS, 0, m_currentCloud.size());

    m_vbo.release();
    m_program->release();
}

void RsLidarWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

// --- 简单的交互控制 ---
void RsLidarWidget::wheelEvent(QWheelEvent *event) {
    m_zOffset += event->angleDelta().y() / 120.0f * 2.0f;
    update();
}

void RsLidarWidget::mousePressEvent(QMouseEvent *event) {
    m_lastMousePos = event->pos();
}

void RsLidarWidget::mouseMoveEvent(QMouseEvent *event) {
    int dx = event->x() - m_lastMousePos.x();
    int dy = event->y() - m_lastMousePos.y();
    if (event->buttons() & Qt::LeftButton) {
        m_rotationX += dy;
        m_rotationY += dx;
    }
    m_lastMousePos = event->pos();
    update();
}
