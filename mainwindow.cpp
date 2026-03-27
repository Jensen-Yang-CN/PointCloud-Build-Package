#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "PcapngReader.h"

#include <QHBoxLayout>
#include <QFileDialog>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

extern "C" {
#include <libavutil/log.h>
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //关闭FFmepg日志输出的，
    //av_log_set_level(AV_LOG_QUIET);

    //pcapng 流式读取过程在独立线程完成读取
    pcapngRead();

    //根据Ip+port分流处理
    ipPortDispatch();

    lidarProcess();

    videoProcess();

    //lidar process


}

MainWindow::~MainWindow() {
    delete ui;
    if(readerThread) {
        readerThread->quit();
        readerThread->wait();
    }
    if(h265ThreadLeft) {
        h265ThreadLeft->quit();
        h265ThreadLeft->wait();
    }
    if(h265ThreadRight) {
        h265ThreadRight->quit();
        h265ThreadRight->wait();
    }
    if(h264ThreadLeft) {
        h264ThreadLeft->quit();
        h264ThreadLeft->wait();
    }
    if(h264ThreadRight) {
        h264ThreadRight->quit();
        h264ThreadRight->wait();
    }
    if(rs201Thread) {
        rs201Thread->quit();
        rs201Thread->wait();
    }


    delete reader;
}
//pcapng 数据读取
void MainWindow::pcapngRead()
{
    reader = new PcapngReader;
    readerThread = new QThread;
    reader->moveToThread(readerThread);
    connect(readerThread, &QThread::finished, reader, &QObject::deleteLater);
    readerThread->start();
}
//IP及端口分流处理
void MainWindow::ipPortDispatch()
{
    udpDispatcher = new UdpDispatcher(this);
    //分流对象创建
    rtpParser1 = new RtpParser(QHostAddress("192.168.58.223"),14256,nullptr);
    rtpParser2 = new RtpParser(QHostAddress("192.168.58.222"),7624,nullptr);
    rsLidarParser213   = new RsLidarParser(QHostAddress("192.168.1.213"),2369,nullptr);
    rsLidarParser214   = new RsLidarParser(QHostAddress("192.168.1.214"),2369,nullptr);
    rsLidarParser201   = new RsLidarParser(QHostAddress("192.168.1.201"),6699,nullptr);
    imuparser          = new IMUParser(QHostAddress("101.1.101.20"),4353,nullptr);
    ls400ParserLeft    = new Ls400Parser(QHostAddress("192.168.1.210"),2369,nullptr);
    ls400ParserRight   = new Ls400Parser(QHostAddress("192.168.1.211"),2369,nullptr);
    rtpParserH264Left  = new RtpParserH264(QHostAddress("192.168.1.170"),20080,nullptr);
    rtpParserH264Right = new RtpParserH264(QHostAddress("192.168.1.170"),20082,nullptr);
    //测试用，后可删除------------------------------------------------------------</
    rtpParserH264Right->setParserName("rtpParserH264Right 20082");//测试用
    rtpParserH264Left->setParserName("rtpParserH264Left 20080");//测试用
    //测试用，后可删除------------------------------------------------------------>/
    //添加分流原则
    //3个Robosense（简称RS）品牌的Helios系列32线雷达
    udpDispatcher->addRule(QHostAddress("192.168.1.201"),6699,rsLidarParser201,&RsLidarParser::inputPacket);
    udpDispatcher->addRule(QHostAddress("192.168.1.213"),6699,rsLidarParser213,&RsLidarParser::inputPacket);
    udpDispatcher->addRule(QHostAddress("192.168.1.214"),6699,rsLidarParser214,&RsLidarParser::inputPacket);
    //2个镭神（简称LS）品牌的LS400S3系列400线雷达
    udpDispatcher->addRule(QHostAddress("192.168.1.210"),2369,ls400ParserLeft, &Ls400Parser::inputPacket);
    udpDispatcher->addRule(QHostAddress("192.168.1.211"),2369,ls400ParserRight,&Ls400Parser::inputPacket);
    //一个船舶姿态记录IMU，
    udpDispatcher->addRule(QHostAddress("101.1.101.20"),4353,imuparser,&IMUParser::inputPacket);
    //2个RGB相机，H265
    udpDispatcher->addRule(QHostAddress("192.168.58.223"), 14256, rtpParser1, &RtpParser::inputPacket);
    udpDispatcher->addRule(QHostAddress("192.168.58.222"), 7624, rtpParser2, &RtpParser::inputPacket);
    //2个红外相机，共用一个IP为
    udpDispatcher->addRule(QHostAddress("192.168.1.170"), 20080, rtpParserH264Left, &RtpParserH264::inputPacket);
    udpDispatcher->addRule(QHostAddress("192.168.1.170"), 20082, rtpParserH264Right, &RtpParserH264::inputPacket);

}
//H265，H264 视频处理

void MainWindow::lidarProcess()
{

    // 1. 初始化成员变量
    rs201Worker = new RsLidarWorker();
    rs201Widget = new RsLidarWidget(); // 建议在 UI 布局中添加此指针
    rs201Thread = new QThread(this);   // 传入 this 方便随主窗口销毁

    // 2. 将 Worker 移动到独立工作线程
    rs201Worker->moveToThread(rs201Thread);

    // --- 关键链路 1: Parser (Pcap读取线程) -> Worker (rs201Thread) ---
    // 由于 Parser 和 Worker 在不同线程，必须用 QueuedConnection
    connect(rsLidarParser201, &RsLidarParser::frameReady,
            rs201Worker, &RsLidarWorker::handleFrame,
            Qt::QueuedConnection);
    //直接测试用
//    connect(rsLidarParser201, &RsLidarParser::frameReady, [=](const LidarFrame &frame){
//        qDebug() << "直接测试：收到帧";
//        rs201Widget->updateCloud(frame.data);
//    });

    // --- 关键链路 2: Worker (rs201Thread) -> Widget (UI线程) ---
    // Worker 处理完点云后发给 UI 显示，同样是跨线程，必须用 QueuedConnection
    connect(rs201Worker, &RsLidarWorker::cloudReady,
            rs201Widget, &RsLidarWidget::updateCloud,
            Qt::QueuedConnection);


    // 3. 资源清理连接：线程停止时自动清理 Worker (可选，如果 Worker 不是 Widget 的子对象)
    connect(rs201Thread, &QThread::finished, rs201Worker, &QObject::deleteLater);

    // 4. 启动工作线程
    rs201Thread->start();

}

void MainWindow::videoProcess()
{

    videoLeft  = new VideoWidget();
    videoRight = new VideoWidget();

    //建立连接，将pcapngReader读取后发给udpDispatcher对象的dispatch
    connect(reader,&PcapngReader::udpPacket,udpDispatcher,&UdpDispatcher::dispatch);
    connect(rtpParserH264Right, &RtpParserH264::rtpPayloadReady,
            rtpParserH264Left,  &RtpParserH264::onExtraConfigReceived);
    //红外相机处理 左侧
    h264ThreadLeft = new QThread();
    h264WorkerLeft = new VideoWorker(nullptr,AV_CODEC_ID_H264);
    h264WorkerLeft->setVideoWorkerName("h264WorkerLeft");
    h264videoLeft = new VideoWidget();
    h264WorkerLeft->moveToThread(h264ThreadLeft);
    connect(h264ThreadLeft, &QThread::started, h264WorkerLeft, &VideoWorker::initDecoder);
    connect(rtpParserH264Left, &RtpParserH264::rtpPayloadReady, h264WorkerLeft, &VideoWorker::handleNal, Qt::QueuedConnection);
    connect(h264WorkerLeft, &VideoWorker::frameReady, h264videoLeft, &VideoWidget::updateFrame, Qt::QueuedConnection);
    h264ThreadLeft->start();

    //红外相机处理 右侧 // 对应 20082 端口的解析器
    h264ThreadRight = new QThread();
    h264WorkerRight = new VideoWorker(nullptr, AV_CODEC_ID_H264);
    h264WorkerRight->setVideoWorkerName("h264WorkerRight");
    h264videoRight = new VideoWidget();
    h264WorkerRight->moveToThread(h264ThreadRight);
    connect(h264ThreadRight, &QThread::started, h264WorkerRight, &VideoWorker::initDecoder);
    connect(rtpParserH264Right, &RtpParserH264::rtpPayloadReady, h264WorkerRight, &VideoWorker::handleNal, Qt::QueuedConnection);
    connect(h264WorkerRight, &VideoWorker::frameReady, h264videoRight, &VideoWidget::updateFrame, Qt::QueuedConnection);
    h264ThreadRight->start();

    //Left
    // 1. 创建对象（注意不要给 worker 指定 parent，否则 moveToThread 会失败）
    h265ThreadLeft = new QThread();
    h265WorkerLeft = new VideoWorker(nullptr,AV_CODEC_ID_HEVC);
    h265WorkerLeft->setVideoWorkerName("h265WorkerLeft");
    // 2. 移动到子线程
    h265WorkerLeft->moveToThread(h265ThreadLeft);
    // 3. 建立连接 (必须在 moveToThread 之后)
    // A. 线程开始时初始化解码器
    connect(h265ThreadLeft, &QThread::started, h265WorkerLeft, &VideoWorker::initDecoder);
    // B. 重要：将 RtpParser 的信号连到 Worker 的处理函数
    // 因为 worker 在子线程，Qt 会自动使用 QueuedConnection，实现异步排队解码
    connect(rtpParser1, &RtpParser::rtpPayloadReady, h265WorkerLeft, &VideoWorker::handleNal, Qt::QueuedConnection);
    // C. 解码出的图像返回 UI
    connect(h265WorkerLeft, &VideoWorker::frameReady, videoLeft, &VideoWidget::updateFrame, Qt::QueuedConnection);
    h265ThreadLeft->start();
    //-------------------------------------------------------------------------------------------
    //Right video
    h265ThreadRight = new QThread();
    h265WorkerRight = new VideoWorker(nullptr,AV_CODEC_ID_HEVC);
    h265WorkerRight->setVideoWorkerName("h265WorkerRight");
    // 2. 移动到子线程
    h265WorkerRight->moveToThread(h265ThreadRight);
    connect(h265ThreadRight, &QThread::started, h265WorkerRight, &VideoWorker::initDecoder);
    // B. 重要：将 RtpParser 的信号连到 Worker 的处理函数
    // 因为 worker 在子线程，Qt 会自动使用 QueuedConnection，实现异步排队解码
    connect(rtpParser2, &RtpParser::rtpPayloadReady, h265WorkerRight, &VideoWorker::handleNal, Qt::QueuedConnection);
    // C. 解码出的图像返回 UI
    connect(h265WorkerRight, &VideoWorker::frameReady, videoRight, &VideoWidget::updateFrame, Qt::QueuedConnection);
    //connect(h265WorkerRight, &VideoWorker::frameReady, videoRight, &VideoWidget::updateFrame, Qt::QueuedConnection);
    h265ThreadRight->start();

    //QThread* imuThread = new QThread();

    auto gridLayout = new QGridLayout(ui->videoContainer);
    gridLayout->addWidget(videoLeft, 0, 0);      // 左上：可见光左
    gridLayout->addWidget(videoRight, 0, 1);     // 右上：可见光右
    gridLayout->addWidget(h264videoLeft, 1, 0);  // 左下：红外左
    gridLayout->addWidget(rs201Widget, 1, 1); // 右下：红外右
    gridLayout->setSpacing(5); // 窗口间距
    ui->videoContainer->setLayout(gridLayout);

    qDebug() << "初始化流水线完成。";

}



void MainWindow::on_pushButton_clicked() {
    // 1. 获取文件列表
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "选择PCAP文件", ".", "PCAP(*.pcap);;All files(*.*)");
    if(fileNames.isEmpty()) return;

    // 2. 异步执行，防止 UI 卡死
    // 使用 QtConcurrent::run 启动一个后台任务
    QtConcurrent::run([this, fileNames]() {
        for(const QString &str : fileNames) {
            qDebug() << "Current Background Thread:" << QThread::currentThread();
            qDebug() << "Processing:" << str;

            // 调用那个带 msleep 控速逻辑的 open 函数
            if(!reader->open(str)) {
                qCritical() << "Failed to open:" << str;
            }
        }
        qDebug() << "All files processed in background.";
    });
}



// 布局
//    auto layout = new QHBoxLayout(ui->videoContainer);
//    layout->addWidget(videoLeft);
//    layout->addWidget(videoRight);
//    layout->addWidget(h264videoLeft);
//    layout->addWidget(h264videoRight);
//    layout->setContentsMargins(0,0,0,0);
//    layout->setStretch(0,1);
//    layout->setStretch(1,1);
//这种写法我不清楚
//    connect(reader, &PcapngReader::udpPacket, udpDispatcher, [=](QHostAddress ip, quint16 port, QByteArray payload){
//        udpDispatcher->dispatch(ip, port, payload);
//    });
