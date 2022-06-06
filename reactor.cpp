#include "reactor.hpp"

void* pollserver(void* void_rea) {
    pReactor rea = (pReactor)void_rea;
    while(1) {
        int pollcount = poll(rea->pfds, rea->size, -1);
        if(pollcount == -1) {
            perror("poll");
        }
        for(int i = 0; i < rea->size; i++) {
            if(rea->pfds[i].revents & POLLIN) {
                rea->funcs[i](&rea->pfds[i].fd);
            }
        }
    }
}

void* newReactor() {
    pReactor rea = (pReactor)malloc(sizeof(Reactor));
    if(rea == NULL) {
        perror("no memory");
    }
    rea->size = 0;
    rea->capacity = 10;
    rea->pfds = (struct pollfd*)malloc(sizeof(struct pollfd) * rea->size);
    if(rea->pfds == NULL) {
        perror("no memory");
    }
    rea->funcs = (func_pointer *)malloc(sizeof(func_pointer) * rea->capacity);
    if(rea->funcs == NULL) {
        perror("no memory");
    }
    pthread_create(&rea->tid, NULL, pollserver, rea);
    return rea;
}

void InstallHandler(pReactor rea, func_pointer func, int fd) {
    if(rea->size == rea->capacity) {
        rea->capacity *= 2;
        rea->pfds = (struct pollfd*)realloc(rea->pfds, sizeof(struct pollfd) * rea->capacity);
        if(rea->pfds == NULL) {
            perror("no memory");
        }
        rea->funcs = (func_pointer *)realloc(rea->funcs, sizeof(func_pointer) * rea->capacity);
        if(rea->funcs == NULL) {
            perror("no memory");
        }
    }
    rea->pfds[rea->size].fd = fd;
    rea->pfds[rea->size].events = POLLIN;
    rea->funcs[rea->size] = func;
    rea->size++;
}

void RemoveHandler(pReactor rea, int fd) {
    for(int i = 0; i < rea->size; i++) {
        if(rea->pfds[i].fd == fd) {
            rea->pfds[i] = rea->pfds[rea->size - 1];
            rea->funcs[i] = rea->funcs[rea->size - 1];
            rea->size--;
            close(fd);
            return;
        }
    }
    perror("not in pfds");
}