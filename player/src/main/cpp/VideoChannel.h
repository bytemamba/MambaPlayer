#ifndef MAMBAPLAYER_VIDEOCHANNEL_H
#define MAMBAPLAYER_VIDEOCHANNEL_H

#include "BaseChannel.h"
#include "AudioChannel.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

typedef void(*RenderCallback)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel {
public:
    VideoChannel(int stream_index, AVCodecContext *codec, AVRational time_base, int fps);

    ~VideoChannel();

    void start();

    void video_decode();

    void video_play();

    void stop();

    void setRenderCallback(RenderCallback callback);

    void setAudioChannel(AudioChannel *channel);

private:
    pthread_t pid_video_decode;
    pthread_t pid_video_play;
    RenderCallback renderCallback;
    int fps;
    AudioChannel *audioChannel = 0;
};

#endif //MAMBAPLAYER_VIDEOCHANNEL_H
