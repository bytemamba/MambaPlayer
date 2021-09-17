#ifndef MAMBAPLAYER_REALPLAYER_H
#define MAMBAPLAYER_REALPLAYER_H

#include <cstring>
#include <pthread.h>
#include "AudioChannel.h"
#include "VideoChannel.h"
#include "JNICallbackHelper.h"
#include "Utils.h"
#include <android/log.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
};

class RealPlayer {
public:
    RealPlayer(const char *data_source, JNICallbackHelper *pHelper);

    ~RealPlayer();

    void prepare();

    void prepareChild();

    void play();

    void playChild();

    void setRenderCallback(RenderCallback callback);

private:
    char *data_source = 0;
    pthread_t pid_prepare;
    pthread_t pid_play;
    AVFormatContext *formatContext = 0;
    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
    JNICallbackHelper *helper = 0;
    bool isPlaying = 0;
    RenderCallback renderCallback;
};

#endif //MAMBAPLAYER_REALPLAYER_H
