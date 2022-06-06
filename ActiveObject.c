#include "ActiveObject.h"

void* ao_thread(void* void_ao) {
    active_object* ao = (active_object*)void_ao;
    while(1) {
        void* front = deQ(ao->que);
        void* from_pre = ao->pre_func(front);
        ao->post_func(from_pre);
    }
}

active_object* newAO(pMyQueue que, void* (*pre_func)(void*), void (*post_func)(void*)) {
    active_object* new_ao = (active_object*)malloc(sizeof(active_object));
    if(new_ao == NULL) {
        perror("no memory");
    }
    new_ao->que = que;
    new_ao->pre_func = pre_func;
    new_ao->post_func = post_func;
    pthread_create(&new_ao->pid, NULL, ao_thread, (void*)new_ao);
    return new_ao;
}

void destroyAO(active_object* ao) {
    destroyQ(ao->que);
    pthread_cancel(ao->pid);
    free(ao);
}