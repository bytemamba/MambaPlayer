#include "VideoChannel.h"

void dropAVFrame(queue<AVFrame *> &q) {
    __android_log_print(ANDROID_LOG_ERROR, "X_TAG", "dropAVFrame");
    if (!q.empty()) {
        AVFrame *frame = q.front();
        BaseChannel::releaseAVFrame(&frame);
        q.pop();
    }
}

void dropAVPackets(queue<AVPacket *> &q) {
    __android_log_print(ANDROID_LOG_ERROR, "X_TAG", "dropAVPackets");
    while (!q.empty()) {
        AVPacket *pkt = q.front();
        if (pkt->flags != AV_PKT_FLAG_KEY) {
            BaseChannel::releaseAVPacket(&pkt);
            q.pop();
        } else {
            break;
        }
    }
}

VideoChannel::VideoChannel(int stream_index, AVCodecContext *codec, AVRational time_base, int fps)
        : BaseChannel(stream_index, codec, time_base), fps(fps) {
    frames.setSyncCallback(dropAVFrame);
    packets.setSyncCallback(dropAVPackets);
}

VideoChannel::~VideoChannel() {
    DELETE(audioChannel)
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
    AVPacket *packet = nullptr;
    while (isPlaying) {
        if (isPlaying && frames.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }
        int ret = packets.getQueueAndDel(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        ret = avcodec_send_packet(codecContext, packet);
        if (ret) {
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret != 0) {
            if (frame) {
                releaseAVFrame(&frame);
            }
            break;
        }
        frames.insertToQueue(frame);
        // 释放空间
        av_packet_unref(packet);
        releaseAVPacket(&packet);
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

    AVFrame *frame = nullptr;
    SwsContext *sws_ctx = sws_getContext(
            codecContext->width, codecContext->height, codecContext->pix_fmt,
            codecContext->width, codecContext->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, nullptr, nullptr, nullptr
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
        // 视频延迟时间
        double extra_delay = frame->repeat_pict / (2 * fps);
        double fps_duration = 1.0 / fps;
        double real_delay = extra_delay + fps_duration;
        // 音视频同步
        double audio_time = audioChannel->audio_time;
        double video_time = frame->best_effort_timestamp * av_q2d(time_base);
        double time_diff = video_time - audio_time;
        if (time_diff > 0) {
            if (time_diff > 1) {
                av_usleep((real_delay * 2) * 1000000);
            } else {
                av_usleep((real_delay + time_diff) * 1000000);
            }
        } else if (time_diff < 0) {
            if (fabs(time_diff) <= 0.05) {
                frames.sync();
                continue;
            }
        }
        // 渲染到 SurfaceView
        renderCallback(*dst_data,
                       codecContext->width, codecContext->height,
                       *dst_line_size);
        av_frame_unref(frame);
        releaseAVFrame(&frame);
    }
    // 释放工作
    av_frame_unref(frame);
    releaseAVFrame(&frame);
    isPlaying = false;
    av_free(&dst_data);
    sws_freeContext(sws_ctx);
}

/**
 * 视频结束
 */
void VideoChannel::stop() {
    isPlaying = false;
    pthread_join(pid_video_decode, nullptr);
    pthread_join(pid_video_play, nullptr);

    packets.setWork(0);
    frames.setWork(0);
    packets.clear();
    frames.clear();
}

void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}

/**
 * 以 AudioChannel 时间为基准
 * @param channel
 */
void VideoChannel::setAudioChannel(AudioChannel *channel) {
    this->audioChannel = channel;
}
