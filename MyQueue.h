#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct Node {
    void* data;
    struct Node *next;
    int fd;
} Node, *pNode;

typedef struct MyQueue {
    pNode front;
    pNode back;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} MyQueue, *pMyQueue;

pNode create_node(void* data, int fd);
void enQ(pMyQueue que, void* data, int fd);
void* deQ(pMyQueue que);
pMyQueue createQ(pthread_mutex_t lock, pthread_cond_t cond);
void destroyQ(pMyQueue que);
