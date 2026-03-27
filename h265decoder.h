#ifndef H265DECODER_H
#define H265DECODER_H

#include <QObject>
#include <QImage>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

class H265Decoder : public QObject
{
    Q_OBJECT
public:
    explicit H265Decoder(QObject *parent = nullptr);
    ~H265Decoder();

    void decode(const QByteArray& nal);

signals:
    void frameDecoded(const QImage& img);
private:
    AVCodecContext* ctx;
    AVFrame* frame;
    AVPacket* pkt;
    SwsContext* sws;
    int width;
    int height;

};

#endif // H265DECODER_H
