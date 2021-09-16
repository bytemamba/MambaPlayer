#include "JNICallbackHelper.h"

JNICallbackHelper::JNICallbackHelper(JavaVM *vm, JNIEnv *env, jobject job) {
    this->vm = vm;
    this->env = env;
    this->job = env->NewGlobalRef(job);

    jclass clz = env->GetObjectClass(job);
    this->method_prepare_id = env->GetMethodID(clz, "onPrepared", "()V");
}

JNICallbackHelper::~JNICallbackHelper() {
    vm = 0;
    env->DeleteGlobalRef(job);
    job = 0;
    env = 0;
}

void JNICallbackHelper::onPrepared(int thread_mode) {
    __android_log_print(ANDROID_LOG_ERROR, "X_TAG", "thread_mode %d" ,thread_mode);
    if (thread_mode == THREAD_MAIN) {
        env->CallVoidMethod(job, method_prepare_id);
    } else if (thread_mode == THREAD_CHILD) {
        JNIEnv *env_child;
        vm->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(job, method_prepare_id);
        vm->DetachCurrentThread();
    }
}
