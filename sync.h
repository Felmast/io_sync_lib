#ifndef MUTEX_SYNC
#define MUTEX_SYNC

// Sync library
// @author Daniela Aguilar
// @author Fabio Calderón
// @author Felipe Ovares
// @author Juleisy Porras

#include <iostream>
#include <mutex>
#include <pthread.h>
#include <vector>
#include <atomic>
#include <condition_variable>

using namespace std;

class Barrier {
public:
    Barrier(int nThreads) {
        mutexs = new pthread_mutex_t [nThreads];
        lock = PTHREAD_COND_INITIALIZER;
        this->nThreads = nThreads;
        for(int i = 0; i<nThreads; i++){
            mutexs[i] = PTHREAD_MUTEX_INITIALIZER;
        }
    }

    Barrier() {
        mutexs = new pthread_mutex_t [1];
        lock = PTHREAD_COND_INITIALIZER;
        this->nThreads = 1;
        mutexs[0] = PTHREAD_MUTEX_INITIALIZER;
    }

    void wait() {
        int ind = count;
        pthread_mutex_lock (&(mutexs[ind]));
        count++;
         if (nThreads <= count) {
            count = 0;
             pthread_cond_broadcast(&lock);
             pthread_mutex_unlock (&(mutexs[ind]));
         } else {
             pthread_cond_wait(&lock, &(mutexs[ind]));
             pthread_mutex_unlock (&(mutexs[ind]));
         }
    }

    ~Barrier(){
            delete[] mutexs;
        }

private:
    int count = 0;
    int nThreads;
    pthread_mutex_t* mutexs;
    pthread_cond_t lock;
};


class sync_monitor{

    private:
        int cond_count = 0;
        bool* cond;
        pthread_mutex_t m_mutex;
        pthread_cond_t* locks;

    public:

        sync_monitor(){
            cond_count = 1;

            cond = new bool[1];

            locks = new pthread_cond_t[1];
            locks[0] = PTHREAD_COND_INITIALIZER;
            cond[0] = true;

            m_mutex = PTHREAD_MUTEX_INITIALIZER;
        }

        sync_monitor(int condition_count){
            condition_count = condition_count <= 0? 1 : condition_count;
            cond_count = condition_count;

            cond = new bool[cond_count];

            locks = new pthread_cond_t[cond_count];
            for(int i = 0; i < cond_count; i++){
                locks[i] = PTHREAD_COND_INITIALIZER;
                cond[i] = true;
            }

            m_mutex = PTHREAD_MUTEX_INITIALIZER;
        }

        ~sync_monitor(){
            delete[] cond;
            delete[] locks;
        }

        static void monitor_lock(sync_monitor* monitor){
          pthread_mutex_lock (&(monitor->m_mutex));
          if(!monitor->cond[0]){
            pthread_cond_wait(&(monitor->locks[0]), &(monitor->m_mutex));
          }
          monitor->cond[0] = false;
        }

        static void monitor_unlock(sync_monitor* monitor){
          monitor_set_condition(monitor, 0, true);
        }

        static void monitor_lock(sync_monitor* monitor, int cond_index){
          pthread_mutex_lock(&(monitor->m_mutex));
          if(!monitor->cond[cond_index]){
            pthread_cond_wait(&(monitor->locks[cond_index]), &(monitor->m_mutex));
          }
          monitor->cond[cond_index] = false;
        }

        static void monitor_unlock(sync_monitor* monitor, int cond_index){
          monitor_set_condition(monitor, cond_index, true);
        }

        static void monitor_set_condition(sync_monitor* monitor, bool val){
          monitor_set_condition(monitor, 0, val);
        }

        static void monitor_set_condition(sync_monitor* monitor, int cond_index, bool val){
          monitor->cond[cond_index] = val;
          if(val){
            pthread_mutex_unlock (&(monitor->m_mutex));
            pthread_cond_signal(&(monitor->locks[cond_index]));
          }
        }

        static bool monitor_get_condition(sync_monitor* monitor){
          return monitor_get_condition(monitor, 0);
        }

        static int monitor_get_condition(sync_monitor* monitor, int cond_index){
          return monitor->cond[cond_index];
        }
};

class Semaphore {
public:
    Semaphore(int count = 0) : count_(count) {
    }

    void wait() {
        unique_lock<mutex> lock(mutex_);
        if (count_ == 0) {
            condition_.wait(lock);
        }
        --count_;
    }

    void signal() {
        ++count_;
        condition_.notify_one();
    }

private:
    atomic<int> count_;
    mutex mutex_;
    condition_variable condition_;
};

#endif
