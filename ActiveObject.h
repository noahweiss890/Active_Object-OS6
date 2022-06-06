#include <stdio.h>
#include "MyQueue.h"


typedef struct active_object {
    pMyQueue que;
    void* (*pre_func)(void*);
    void (*post_func)(void*);
    pthread_t pid;
} active_object;

void* ao_thread(void* void_ao);
active_object* newAO(pMyQueue que, void* (*pre_func)(void*), void (*post_func)(void*));
void destroyAO(active_object* ao);
