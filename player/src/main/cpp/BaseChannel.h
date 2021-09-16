#ifndef MAMBAPLAYER_BASECHANNEL_H
#define MAMBAPLAYER_BASECHANNEL_H

#include "SafeQueue.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

class BaseChannel {
public:
    int stream_index;
    bool isPlaying;
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
    AVCodecContext *codecContext = 0;

    BaseChannel(int stream_index, AVCodecContext *codecContext) :
            stream_index(stream_index), codecContext(codecContext) {
        packets.setReleaseCallback(releaseAVPacket);
        frames.setReleaseCallback(releaseAVFrame);
    }

    virtual ~BaseChannel() {
        packets.clear();
        frames.clear();
    }

    static void releaseAVPacket(AVPacket **p) {
        if (p) {
            av_packet_free(p);
            *p = 0;
        }
    }

    static void releaseAVFrame(AVFrame **f) {
        if (f) {
            av_frame_free(f);
            *f = 0;
        }
    }
};

#endif //MAMBAPLAYER_BASECHANNEL_H
