#include "VideoChannel.h"
#include "RealPlayer.h"

VideoChannel::VideoChannel(int stream_index, AVCodecContext *codec)
        : BaseChannel(stream_index, codec) {

}

VideoChannel::~VideoChannel() {

}

void *task_video_decode(void *args) {
    auto channel = static_cast<VideoChannel *> (args);
    channel->video_decode();
    return 0;
}

void *task_video_play(void *args) {
    auto channel = static_cast<VideoChannel *>(args);
    channel->video_play();
    return 0;
}

/**
 * 视频开始
 */
void VideoChannel::start() {
    isPlaying = true;
    // 队列工作状态
    packets.setWork(1);
    frames.setWork(1);
    // 开启编码线程
    pthread_create(&pid_video_decode, 0, task_video_decode, this);
    // 开启播放线程
    pthread_create(&pid_video_play, 0, task_video_play, this);
}

/**
 * 视频解码
 *
 * 压缩包解码成原始包
 */
void VideoChannel::video_decode() {
    AVPacket *packet = av_packet_alloc();
    while (isPlaying) {
        int ret = packets.getQueueAndDel(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        ret = avcodec_send_packet(codecContext, packet);
        releaseAVPacket(&packet);
        if (ret) {
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret != 0) {
            break;
        }
        frames.insertToQueue(frame);
    }
    releaseAVPacket(&packet);
}

/**
 * 视频播放
 *
 * 取原始包、转换格式播放
 */
void VideoChannel::video_play() {
    // RGBA
    uint8_t *dst_data[4];
    int dst_line_size[4];
    av_image_alloc(dst_data, dst_line_size,
                   codecContext->width, codecContext->height,
                   AV_PIX_FMT_RGBA, 1);

    AVFrame *frame = 0;
    SwsContext *sws_ctx = sws_getContext(
            codecContext->width, codecContext->height, codecContext->pix_fmt,
            codecContext->width, codecContext->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, NULL, NULL, NULL
    );

    while (isPlaying) {
        int ret = frames.getQueueAndDel(frame);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        // 格式转换 YUV - RGBA
        sws_scale(sws_ctx, frame->data, frame->linesize,
                  0, codecContext->height,
                  dst_data, dst_line_size);
        // 渲染到 SurfaceView
        renderCallback(*dst_data,
                       codecContext->width, codecContext->height,
                       *dst_line_size);
        releaseAVFrame(&frame);
    }
    // 释放工作
    releaseAVFrame(&frame);
    isPlaying = false;
    av_free(&dst_data);
    sws_freeContext(sws_ctx);
}

/**
 * 视频结束
 */
void VideoChannel::stop() {

}

void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}
