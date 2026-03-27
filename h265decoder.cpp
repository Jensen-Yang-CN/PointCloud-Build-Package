#include "h265decoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <QDebug>

H265Decoder::H265Decoder(QObject *parent)
    : QObject(parent), ctx(nullptr), frame(nullptr), pkt(nullptr), sws(nullptr), width(0), height(0)
{
    // 1. 查找解码器
    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
    if (!codec) {
        qCritical() << "未找到 H.265 解码器";
        return;
    }

    // 2. 分配上下文和结构体
    ctx = avcodec_alloc_context3(codec);
    pkt = av_packet_alloc();
    frame = av_frame_alloc();

    // 3. 打开解码器
    if (avcodec_open2(ctx, codec, NULL) < 0) {
        qCritical() << "无法打开解码器";
    }
}

H265Decoder::~H265Decoder() {
    if (ctx) avcodec_free_context(&ctx);
    if (frame) av_frame_free(&frame);
    if (pkt) av_packet_free(&pkt);
    if (sws) sws_freeContext(sws);
}

void H265Decoder::decode(const QByteArray& nal) {
    if (!ctx || nal.isEmpty()) return;
    if (nal.isEmpty()) return;

    qDebug()<< "H265Decoder-->decode";
    pkt->data = (uint8_t*)nal.data();
    pkt->size = nal.size();

    int ret = avcodec_send_packet(ctx, pkt);

    // 如果返回无效数据错误，通常是还没刷入参数
    if (ret == AVERROR_INVALIDDATA) {
        // 可以在这里静默处理，直到收到 SPS/PPS 为止
        return;
    }

    // 装载数据到 Packet
    pkt->data = (uint8_t*)nal.data();
    pkt->size = nal.size();

    // 发送数据到解码器
    if (ret < 0) return;

    // 循环获取解码后的 Frame
    while (ret >= 0) {
        ret = avcodec_receive_frame(ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
        if (ret < 0) return;

        // 检查并更新转换器（如果分辨率变化）
        if (width != frame->width || height != frame->height) {
            width = frame->width;
            height = frame->height;
            sws = sws_getCachedContext(sws, width, height, (AVPixelFormat)frame->format,
                                       width, height, AV_PIX_FMT_RGB32,
                                       SWS_BICUBIC, NULL, NULL, NULL);
        }

        // 转换为 QImage
        QImage img(width, height, QImage::Format_RGB32);
        uint8_t* dest[] = { img.bits() };
        int dest_linesize[] = { (int)img.bytesPerLine() };

        sws_scale(sws, frame->data, frame->linesize, 0, height, dest, dest_linesize);

        // 发射信号显示
        emit frameDecoded(img.copy());
    }
}
