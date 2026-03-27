#include "ffmpeg_wrapper.h"

#include "ffmpeg_wrapper.h"
#include <QDebug>

FFmpegDecoder::FFmpegDecoder()
{
    pkt = av_packet_alloc();
    frame = av_frame_alloc();
}

FFmpegDecoder::~FFmpegDecoder()
{
    if (ctx) avcodec_free_context(&ctx);
    if (frame) av_frame_free(&frame);
    if (pkt) av_packet_free(&pkt);
    if (swsCtx) sws_freeContext(swsCtx);
}

bool FFmpegDecoder::init(int codecId)
{
    codec = avcodec_find_decoder((AVCodecID)codecId);
    if (!codec) {
        qDebug() << "codec not found";
        return false;
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx) return false;

    if (avcodec_open2(ctx, codec, nullptr) < 0) {
        qDebug() << "avcodec_open2 failed";
        return false;
    }

    qDebug() << "FFmpeg decoder init success";
    return true;
}

bool FFmpegDecoder::isValid() const {
    return ctx != nullptr;
}

QImage FFmpegDecoder::decodeFrame(const QByteArray &data)
{
    if (!ctx) return QImage();

    av_packet_unref(pkt);
    av_new_packet(pkt, data.size());
    memcpy(pkt->data, data.data(), data.size());
    pkt->size = data.size();

    int ret = avcodec_send_packet(ctx, pkt);
    if (ret < 0) return QImage();

    ret = avcodec_receive_frame(ctx, frame);
    if (ret < 0) return QImage();

    if (!swsCtx) {
        swsCtx = sws_getContext(
            frame->width, frame->height, (AVPixelFormat)frame->format,
            frame->width, frame->height, AV_PIX_FMT_RGB32,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
    }

    QImage img(frame->width, frame->height, QImage::Format_RGB32);

    uint8_t *dst[4] = { img.bits(), nullptr, nullptr, nullptr };
    int dstLinesize[4] = { img.bytesPerLine(), 0, 0, 0 };

    sws_scale(
        swsCtx,
        frame->data,
        frame->linesize,
        0,
        frame->height,
        dst,
        dstLinesize
    );

    return img;
}
