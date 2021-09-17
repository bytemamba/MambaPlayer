#include "AudioChannel.h"

AudioChannel::AudioChannel(int stream_index, AVCodecContext *codec)
        : BaseChannel(stream_index, codec) {
    // 音频三要素
    // 通道数、位深、采样率
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;
    // 堆区开辟空间
    out_buffers_size = out_channels * out_sample_size * out_sample_rate;
    out_buffers = static_cast<uint8_t *>(malloc(out_buffers_size));
    // 音频重采样初始化
    swr_ctx = swr_alloc_set_opts(0,
                                 AV_CH_LAYOUT_STEREO,
                                 AV_SAMPLE_FMT_S16,
                                 out_sample_rate,
                                 codecContext->channels,
                                 codecContext->sample_fmt,
                                 codecContext->sample_rate,
                                 0, 0);
    swr_init(swr_ctx);
}

AudioChannel::~AudioChannel() {

}

void *task_audio_decode(void *args) {
    auto *audio = static_cast<AudioChannel *> (args);
    audio->audio_decode();
    return 0;
}

void *task_audio_play(void *args) {
    auto *audio = static_cast<AudioChannel *>(args);
    audio->audio_play();
    return 0;
}

/**
 * 开启线程
 */
void AudioChannel::start() {
    isPlaying = 1;

    packets.setWork(1);
    frames.setWork(1);

    pthread_create(&pid_audio_decode, 0, task_audio_decode, this);
    pthread_create(&pid_audio_play, 0, task_audio_play, this);
}

void AudioChannel::stop() {

}

/**
 * 音频解码
 *
 * 将原始包丢入队列
 */
void AudioChannel::audio_decode() {
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

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *args) {
    auto *audio_channel = static_cast<AudioChannel *>(args);
    int pcm_size = audio_channel->getPCM();
    (*bq)->Enqueue(
            bq,
            audio_channel->out_buffers,
            pcm_size
    );
}

/**
 * 播放音频
 */
void AudioChannel::audio_play() {
    // 创建引擎并获取接口
    SLresult result = slCreateEngine(&engineObject, 0,
                                     NULL, 0,
                                     NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 设置混音器
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject,
                                                 0, 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 缓冲队列
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq =
            {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    // PCM 数据格式
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                                   SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};
    // 配置音轨
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    // 创建播放器
    result = (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject,
                                                   &audioSrc, &audioSnk,
                                                   1, ids, req);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 设置播放回调函数
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);
    // 设置播放状态
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    // 手动激活
    bqPlayerCallback(bqPlayerBufferQueue, this);
}

/**
 * 获取数据大小
 * @return
 */
int AudioChannel::getPCM() {
    int pcm_data_size = 0;
    AVFrame *frame = 0;
    int ret = frames.getQueueAndDel(frame);
    if (!isPlaying) {
        return pcm_data_size;
    }
    if (!ret) {
        return pcm_data_size;
    }
    // 开始重采样
    // 获取单通道的样本数
    int dst_nb_samples = av_rescale_rnd(
            swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples,
            out_sample_rate,
            codecContext->sample_rate,
            AV_ROUND_UP
    );
    int samples_per_channel = swr_convert(
            swr_ctx,
            &out_buffers,
            dst_nb_samples,
            (const uint8_t **) frame->data,
            frame->nb_samples
    );
    pcm_data_size = samples_per_channel * out_sample_size * out_channels;
    return pcm_data_size;
}
