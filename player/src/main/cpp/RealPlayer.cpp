#include "RealPlayer.h"

RealPlayer::RealPlayer(const char *data_source, JNICallbackHelper *helper) {
    this->data_source = new char[strlen(data_source) + 1];
    strcpy(this->data_source, data_source);
    this->helper = helper;
}

RealPlayer::~RealPlayer() {
    if (data_source) {
        delete data_source;
        data_source = nullptr;
    }
    if (helper) {
        delete helper;
        helper = nullptr;
    }
}

void *task_prepare(void *args) {
    auto *player = static_cast<RealPlayer *>(args);
    player->prepareChild();
    return nullptr;
}

void RealPlayer::prepare() {
    pthread_create(&pid_prepare, 0, task_prepare, this);
}

/**
 * 子线程准备数据
 */
void RealPlayer::prepareChild() {
    // 创建 format 上下文
    formatContext = avformat_alloc_context();
    AVDictionary *avDictionary = nullptr;
    av_dict_set(&avDictionary, "timeout", "5000000", 0);
    // 打开文件
    int ret = avformat_open_input(&formatContext, data_source, nullptr, &avDictionary);
    av_dict_free(&avDictionary);
    if (ret) {
        // 错误回调
        char *errMsg = av_err2str(ret);
        __android_log_print(ANDROID_LOG_ERROR, "X_TAG", "onPrepared failed %s", errMsg);
        return;
    }

    ret = avformat_find_stream_info(formatContext, nullptr);
    if (ret < 0) {
        return;
    }

    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream *stream = formatContext->streams[i];
        AVCodecParameters *parameters = stream->codecpar;
        // 根据流参数获取编解码器
        AVCodec *codec = avcodec_find_decoder(parameters->codec_id);
        if (!codec) {
            return;
        }
        // 创建编解码上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            return;
        }
        // 设置参数到上下文
        ret = avcodec_parameters_to_context(codecContext, parameters);
        if (ret < 0) {
            return;
        }
        // 打开编解码器
        ret = avcodec_open2(codecContext, codec, nullptr);
        if (ret) {
            return;
        }
        AVRational time_base = stream->time_base;
        if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO) {
            audioChannel = new AudioChannel(i, codecContext, time_base);
        } else if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO) {
            if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                continue;
            }
            AVRational fps_rational = stream->avg_frame_rate;
            int fps = av_q2d(fps_rational);
            videoChannel = new VideoChannel(i, codecContext, time_base, fps);
            videoChannel->setRenderCallback(renderCallback);
        }
    }

    if (!audioChannel || !videoChannel) {
        return;
    }

    // 准备完成，回调上层数据
    if (helper) {
        helper->onPrepared(THREAD_CHILD);
    }
}

void *task_play(void *args) {
    auto *player = static_cast<RealPlayer *>(args);
    player->playChild();
    return nullptr;
}

void RealPlayer::play() {
    isPlaying = true;

    if (videoChannel) {
        videoChannel->setAudioChannel(audioChannel);
        videoChannel->start();
    }
    if (audioChannel) {
        audioChannel->start();
    }

    pthread_create(&pid_play, nullptr, task_play, this);
}

void RealPlayer::playChild() {
    while (isPlaying) {
        if (videoChannel && videoChannel->packets.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }
        if (audioChannel && audioChannel->packets.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }

        AVPacket *packet = av_packet_alloc();
        int ret = av_read_frame(formatContext, packet);
        if (!ret) {
            if (videoChannel && packet->stream_index == videoChannel->stream_index) {
                videoChannel->packets.insertToQueue(packet);
            } else if (audioChannel && packet->stream_index == audioChannel->stream_index) {
                audioChannel->packets.insertToQueue(packet);
            }
        } else if (ret == AVERROR_EOF) {
            if (videoChannel->packets.empty() && audioChannel->packets.empty()) {
                break;
            }
        } else {
            break;
        }
    }
    isPlaying = false;
    videoChannel->stop();
    audioChannel->stop();
}

void RealPlayer::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}
