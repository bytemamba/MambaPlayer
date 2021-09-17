#ifndef MAMBAPLAYER_AUDIOCHANNEL_H
#define MAMBAPLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include <libswresample/swresample.h>
}

class AudioChannel : public BaseChannel {
public:
    AudioChannel(int stream_index, AVCodecContext *codec);

    ~AudioChannel();

    void start();

    void stop();

    void audio_decode();

    void audio_play();

private:
    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;
    SLObjectItf engineObject = 0;
    SLEngineItf engineInterface = 0;
    SLObjectItf outputMixObject = 0;
    SLObjectItf bqPlayerObject = 0;
    SLPlayItf bqPlayerPlay = 0;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = 0;

public:
    int out_channels;
    int out_sample_size;
    int out_sample_rate;
    int out_buffers_size;
    uint8_t *out_buffers = 0;
    SwrContext *swr_ctx = 0;

    int getPCM();
};


#endif //MAMBAPLAYER_AUDIOCHANNEL_H
