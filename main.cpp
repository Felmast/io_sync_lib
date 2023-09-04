#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <unistd.h>
#include <windows.h>
#include "sync.h"
#include <math.h>


using namespace std;

void printList(int* list, int start, int end){
    for(int i=start; i <= end; i++){
        std::cout << list[i];
        if(i < end)
            std::cout << ", ";
    }
    std::cout << "\n";
}

/*sync_monitor* m;
sync_barrier* mb;
sync_semaphor* sm;

void printList(int* list, int start, int end){
    for(int i=start; i <= end; i++){
        std::cout << list[i];
        if(i < end)
            std::cout << ", ";
    }
    std::cout << "\n";
}


static void* foo(void* arg){
    sync_monitor::monitor_lock(m);
    std::cout << "World 1!\n";
    sync_monitor::monitor_unlock(m);
    pthread_exit(0);
}

static void* foo2(void* arg){
    sync_monitor::monitor_lock(m);
    std::cout << "World 2!\n";
    sync_monitor::monitor_unlock(m);
    pthread_exit(0);
}


static void* foo3(void* arg){
    sync_monitor::monitor_lock(m, 1);
    std::cout << "world 3!\n";
    sync_monitor::monitor_unlock(m, 1);
    //Sleep(1000);
    sync_monitor::monitor_unlock(m);
    pthread_exit(0);
}

static void* foo_barrier1(void* arg){
    int index = sync_barrier::get_current_waiting(mb);
    sync_barrier::barrier_lock(mb);
    std::cout << "Finished: " << index << "\n";
    pthread_exit(0);
}

static void* foo_semaphor(void* arg){
    bool available = sync_semaphor::semaphor_available(sm);
    sync_semaphor::semaphor_acquire(sm);
    std::cout << "Semaphor: " << (int)arg << ", " << available << "\n";
    sync_semaphor::semaphor_release(sm);
    pthread_exit(0);
}
*/


struct merge_list{
    int* list;
    int size;
};

struct merge_list_aux{
    merge_list* ml;
    int start;
    int end;
    Barrier* barrier;
};

static void* merge_sort_aux(void* args){
    merge_list_aux* l = (merge_list_aux*)args;
    int mid = (l->start + l->end) / 2 + 1;

    //If "end" is even, it means that size is odd and this is the last sub-array
    //binary AND of any int with "1" will return either 0 or 1, if the first bit is active, which also means that is odd
    //Then "not" that bit, because !0 = true
    mid = (!(l->end & 1))? mid-1  : mid;

    //std::cout << l->start << ", " << mid << ", " << l->end << "\n";
    //printList(l->ml->list, l->start, l->end);

    int tmp[l->end - l->start + 1];
    int pos = 0;
    int sa1 = l->start;
    int sa2 = mid;

    while(sa1 < mid && sa2 <= l->end){
        if(l->ml->list[sa1] <= l->ml->list[sa2]){
            tmp[pos] = l->ml->list[sa1];
            sa1++;
        }else{
            tmp[pos] = l->ml->list[sa2];
            sa2++;
        }
        pos++;
    }
    while(sa1 < mid){
        tmp[pos] = l->ml->list[sa1];
        sa1++;
        pos++;
    }
    while(sa2 <= l->end){
        tmp[pos] = l->ml->list[sa2];
        sa2++;
        pos++;
    }

    pos = 0;
    while(l->start <= l->end){
        l->ml->list[l->start] = tmp[pos];
        l->start++;
        pos++;
    }

    //Wait for other threads
    l->barrier->wait();
    pthread_exit(0);
}


static void* merge_sort(void* args){
    merge_list* l = (merge_list*)args;
    int threads = l->size;

    int barrier_num = 0;
    Barrier** barriers = (Barrier**) ( new Barrier[(int)(ceil(sqrt(l->size))) + 1] );

    //Special case for odds sizes
    if( l->size > 1 && l->size % 2 == 1){
        int t1, t2, t3;
        t1 = l->list[l->size-1];
        t2 = l->list[l->size-2];
        t3 = l->list[l->size-3];

        l->list[l->size-1] = t1 > t2? (t1 > t3? t1 : t2) : (t2 > t3? t2 : t3); //Biggest of 3
        l->list[l->size-3] = t1 < t2? (t1 < t3? t1 : t3) : (t2 < t3? t2 : t3); //Smallest of 3
        l->list[l->size-2] = t1 + t2 + t3 - l->list[l->size-1] - l->list[l->size-3]; //The other one
    }

    int i = 0;
    int size = l->size - 1;
    threads = threads / 2;
    while(threads > 1){
        barriers[barrier_num] = new Barrier(threads+1);
        pthread_t tid;
        i = 0;
        size = l->size / threads;
        std::cout << "Size: " << size << ", " << threads << "\n";
        for(int k = 0; k < threads; k++){
            merge_list_aux* data = new merge_list_aux();
            data->ml = l;
            data->start = i;
            data->end = i + size - 1 + ((k == threads-1) && (l->size % 2 == 1));
            data->barrier = barriers[barrier_num];

            pthread_create(&tid, nullptr, &merge_sort_aux, (void*)data);

            i = i + size;
        }

        barriers[barrier_num]->wait();
        barrier_num++;
        threads = threads / 2;
    }

    barriers[barrier_num] = new Barrier(2);
    pthread_t tid;
    merge_list_aux* data = new merge_list_aux();
    data->ml = l;
    data->start = 0;
    data->end = l->size - 1;
    data->barrier = barriers[barrier_num];

    pthread_create(&tid, nullptr, &merge_sort_aux, (void*)data);
    barriers[barrier_num]->wait();
    pthread_join(tid, nullptr);

    for(int i = 0; i <= barrier_num; i ++)
        delete barriers[i];
    delete barriers;

    pthread_exit(0);
}


int main() {
    /*m = new sync_monitor(10);

    //MONITOR
    sync_monitor::monitor_lock(m);

    pthread_t t1, t2, t3;

    pthread_create(&t1, nullptr, &foo, nullptr);
    pthread_create(&t2, nullptr, &foo2, nullptr);
    pthread_create(&t3, nullptr, &foo3, nullptr);

    std::cout << "Hello world!\n";

    sync_monitor::monitor_set_condition(m, 1, true);

    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);
    pthread_join(t3, nullptr);

    std::cout << "Bye world!\n";

    //BARRIER
    mb = new sync_barrier(10);
    pthread_t barrier_threads[10];
    for(int i = 0; i < 9; i++){
        pthread_create(&barrier_threads[i], nullptr, &foo_barrier1, nullptr);
    }
    std::cout << "\nBarrier\n";
    sync_barrier::barrier_lock(mb);

    for(int i = 0; i < 10; i++){
        pthread_join(barrier_threads[i], nullptr);
    }

    //SEMPAHOR
    std::cout << "\n\nSemaphor\n";
    sm = new sync_semaphor(3);
    pthread_t semaphor_threads[10];

    for(int i = 0; i < 10; i++){
        pthread_create(&semaphor_threads[i], nullptr, &foo_semaphor, (void*)i);
    }

    for(int i = 0; i < 10; i++){
        pthread_join(semaphor_threads[i], nullptr);
    }*/

    //MERGE SORT
    std::cout << "\n\nMerge Sort\n";
    int lista[] = {9, 6, 12, 5, 1, 24, 3, 99, 7,7,9,23,4,6,2345,25,9};
    merge_list mlist;
    mlist.list = lista;
    mlist.size = 17;

    printList(mlist.list, 0, mlist.size-1);

    pthread_t megresort;
    pthread_create(&megresort, nullptr, &merge_sort, (void*)&mlist);
    pthread_join(megresort, nullptr);

    printList(mlist.list, 0, mlist.size-1);


    //delete m;
    //delete mb;
    //delete sm;
    return 0;
}
