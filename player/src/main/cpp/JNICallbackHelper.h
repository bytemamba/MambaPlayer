#ifndef MAMBAPLAYER_JNICALLBACKHELPER_H
#define MAMBAPLAYER_JNICALLBACKHELPER_H

#include <jni.h>
#include "JNICallbackHelper.h"
#include "Utils.h"
#include <android/log.h>

class JNICallbackHelper {
private:
    JavaVM *vm = 0;
    JNIEnv *env = 0;
    jobject job;
    jmethodID method_prepare_id;
    jmethodID method_progress_id;

public:
    JNICallbackHelper(JavaVM *vm, JNIEnv *env, jobject job);

    ~JNICallbackHelper();

    void onPrepared(int thread_mode);

    void onProgress(int thread_mode, int progress);
};


#endif //MAMBAPLAYER_JNICALLBACKHELPER_H
