#include "RealPlayer.h"

RealPlayer::RealPlayer(const char *data_source, JNICallbackHelper *helper) {
    this->data_source = new char[strlen(data_source) + 1];
    strcpy(this->data_source, data_source);
    this->helper = helper;
}

RealPlayer::~RealPlayer() {
    if (data_source) {
        delete data_source;
    }
}

void *task_prepare(void *args) {
    auto *player = static_cast<RealPlayer *>(args);
    player->prepareChild();
    return 0;
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
    AVDictionary *avDictionary = 0;
    av_dict_set(&avDictionary, "timeout", "5000000", 0);
    // 打开文件
    int ret = avformat_open_input(&formatContext, data_source, 0, &avDictionary);
    av_dict_free(&avDictionary);
    if (ret) {
        __android_log_print(ANDROID_LOG_ERROR, "X_TAG", "onPrepared open failed");
        return;
    }

    ret = avformat_find_stream_info(formatContext, 0);
    if (ret < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "X_TAG", "onPrepared find failed");
        return;
    }

    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream *stream = formatContext->streams[i];
        AVCodecParameters *parameters = stream->codecpar;
        // 根据流参数获取编解码器
        AVCodec *codec = avcodec_find_decoder(parameters->codec_id);
        // 创建编解码上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            __android_log_print(ANDROID_LOG_ERROR, "X_TAG", "onPrepared codecContext failed");
            return;
        }
        // 设置参数到上下文
        ret = avcodec_parameters_to_context(codecContext, parameters);
        if (ret < 0) {
            __android_log_print(ANDROID_LOG_ERROR, "X_TAG", "onPrepared avcodec_parameters_to_context failed");
            return;
        }
        // 打开编解码器
        ret = avcodec_open2(codecContext, codec, 0);
        if (ret) {
            __android_log_print(ANDROID_LOG_ERROR, "X_TAG", "onPrepared avcodec_open2 failed");
            return;
        }
        if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO) {
            audioChannel = new AudioChannel();
        } else if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO) {
            videoChannel = new VideoChannel();
        }
    }

    __android_log_print(ANDROID_LOG_ERROR, "X_TAG", "open success");

    if (!audioChannel || !videoChannel) {
        return;
    }

    // 准备完成，回调上层数据
    if (helper) {
        helper->onPrepared(THREAD_CHILD);
    }
}