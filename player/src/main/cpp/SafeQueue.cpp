#include "SafeQueue.h"

template<typename T>
SafeQueue<T>::SafeQueue() {
    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&cond, 0);
}

template<typename T>
SafeQueue<T>::~SafeQueue() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

template<typename T>
void SafeQueue<T>::insertToQueue(T value) {
    pthread_mutex_lock(&mutex);

    if (work) {
        queue.push(value);
        pthread_cond_signal(&cond);
    } else {
        if (releaseCallback) {
            releaseCallback(&value);
        }
    }

    pthread_mutex_unlock(&mutex);
}

template<typename T>
int SafeQueue<T>::getQueueAndDel(T &value) {
    int ret = 0;

    pthread_mutex_lock(&mutex);

    while (work && queue.empty()) {
        pthread_cond_wait(&cond, &mutex);
    }

    if (!queue.empty()) {
        value = queue.front();
        queue.pop();
        ret = 1;
    }

    pthread_mutex_unlock(&mutex);

    return ret;
}

template<typename T>
void SafeQueue<T>::setWork(int work) {
    pthread_mutex_lock(&mutex);

    this->work = work;

    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

template<typename T>
int SafeQueue<T>::empty() {
    return queue.empty();
}

template<typename T>
int SafeQueue<T>::size() {
    return queue.size();
}

template<typename T>
void SafeQueue<T>::clear() {
    pthread_mutex_lock(&mutex); // 多线程的访问（先锁住）

    unsigned int size = queue.size();

    for (int i = 0; i < size; ++i) {
        T value = queue.front();
        if (releaseCallback) {
            releaseCallback(&value);
        }
        queue.pop();
    }

    pthread_mutex_unlock(&mutex);
}

template<typename T>
void SafeQueue<T>::setReleaseCallback(SafeQueue::ReleaseCallback releaseCallback) {
    this->releaseCallback = releaseCallback;
}

