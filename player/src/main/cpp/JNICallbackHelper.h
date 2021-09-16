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

public:
    JNICallbackHelper(JavaVM *vm, JNIEnv *env, jobject job);

    ~JNICallbackHelper();

    void onPrepared(int thread_mode);
};


#endif //MAMBAPLAYER_JNICALLBACKHELPER_H
