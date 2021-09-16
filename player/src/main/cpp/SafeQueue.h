#ifndef MAMBAPLAYER_SAFEQUEUE_H
#define MAMBAPLAYER_SAFEQUEUE_H

#include <queue>
#include <pthread.h>

using namespace std;

// 泛型
template<typename T>

class SafeQueue {
private:
    typedef void (*ReleaseCallback)(T *);

    int work;
    queue <T> queue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    ReleaseCallback releaseCallback;

public:
    SafeQueue();

    ~SafeQueue();

    void insertToQueue(T value);

    int getQueueAndDel(T &value);

    void setWork(int work);

    int empty();

    int size();

    void clear();

    void setReleaseCallback(ReleaseCallback releaseCallback);
};

#endif //MAMBAPLAYER_SAFEQUEUE_H
