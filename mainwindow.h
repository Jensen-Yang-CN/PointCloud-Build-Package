#pragma once
#include <QMainWindow>
#include <QThread>
#include "PcapngReader.h"
#include "UdpDispatcher.h"
#include "rtpparser.h"
#include "VideoWorker.h"
#include "VideoWidget.h"
#include "RsLidarParser.h"
#include "IMUParser.h"
#include "Ls400Parser.h"
#include "RtpParserH264.h"
#include "RsLidarWorker.h"
#include "RsLidarWidget.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void pcapngRead();
    void ipPortDispatch();
    void videoProcess();
    void lidarProcess();
private:
    Ui::MainWindow *ui;
    PcapngReader *reader;
    QThread *readerThread;
    UdpDispatcher *udpDispatcher;
    RtpParser *rtpParser1;
    RtpParser *rtpParser2;
    RtpParserH264*  rtpParserH264Left;
    RtpParserH264*  rtpParserH264Right;

    VideoWorker *h265WorkerLeft;
    QThread *h265ThreadLeft;
    VideoWidget *videoLeft;

    VideoWorker *h265WorkerRight;
    QThread *h265ThreadRight;
    VideoWidget *videoRight;

    VideoWorker *h264WorkerLeft;
    QThread *h264ThreadLeft;
    VideoWidget *h264videoLeft;

    VideoWorker *h264WorkerRight;
    QThread *h264ThreadRight;
    VideoWidget *h264videoRight;


    RsLidarParser* rsLidarParser213;
    RsLidarParser* rsLidarParser214;
    RsLidarParser* rsLidarParser201;
    IMUParser* imuparser;
    Ls400Parser* ls400ParserLeft;
    Ls400Parser* ls400ParserRight;

    RsLidarWorker* rs201Worker;
    RsLidarWidget* rs201Widget;
    QThread *rs201Thread;






private slots:
    void on_pushButton_clicked();
};
