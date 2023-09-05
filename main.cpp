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
    return 0;
}
