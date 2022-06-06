#include "MyQueue.h"


pNode create_node(void* data, int fd) {
    pNode new_node = (pNode)malloc(sizeof(Node));
    if(new_node == NULL) {
        perror("no memory");
    }
    new_node->data = data;
    new_node->next = NULL;
    new_node->fd = fd;
    return new_node;
}

void enQ(pMyQueue que, void* data, int fd) {
    pNode new_node = create_node(data, fd);
    pthread_mutex_lock(&que->lock);
    if(que->front == NULL) {
        que->front = new_node;
        que->back = new_node;
    }
    else {
        que->back->next = new_node;
        que->back = new_node;
    }
    pthread_cond_signal(&que->cond);
    pthread_mutex_unlock(&que->lock);
}

void* deQ(pMyQueue que) {
    pthread_mutex_lock(&que->lock);
    if(que->front == NULL) {
        pthread_cond_wait(&que->cond, &que->lock);
    }
    pNode to_del = que->front;
    que->front = que->front->next;
    if(que->front == NULL) {
        que->back = NULL;
    }
    pthread_mutex_unlock(&que->lock);
    return to_del;
}

pMyQueue createQ(pthread_mutex_t lock, pthread_cond_t cond) {
    pMyQueue new_queue = (pMyQueue)malloc(sizeof(MyQueue));
    if(new_queue == NULL) {
        perror("no memory");
    }
    new_queue->front = NULL;
    new_queue->back = NULL;
    new_queue->lock = lock;
    new_queue->cond = cond;
    return new_queue;
}

void destroyQ(pMyQueue que) {
    pthread_mutex_lock(&que->lock);
    while(que->front != NULL) {
        deQ(que);
    }
    free(que);
    pthread_mutex_unlock(&que->lock);
}