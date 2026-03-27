#ifndef FFMPEGDECODER_H
#define FFMPEGDECODER_H


#include <QImage>
#include <QByteArray>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class FFmpegDecoder {
public:
    FFmpegDecoder();
    ~FFmpegDecoder();

    bool init(int codecId);
    QImage decodeFrame(const QByteArray &data);

    bool isValid() const;

private:
    AVCodecContext *ctx = nullptr;
    const AVCodec *codec = nullptr;
    AVPacket *pkt = nullptr;
    AVFrame *frame = nullptr;
    SwsContext *swsCtx = nullptr;
};

#endif // FFMPEGDECODER_H
