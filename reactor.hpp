#include <iostream>
#include <thread>
#include <poll.h>
#include <unistd.h>

typedef void (*func_pointer)(void*);

typedef struct Reactor {
    struct pollfd *pfds;
    func_pointer *funcs;
    int size;
    int capacity;
    pthread_t tid;
} Reactor, *pReactor;

void* pollserver(void* void_rea);
void* newReactor();
void InstallHandler(pReactor rea, func_pointer func, int fd);
void RemoveHandler(pReactor rea, int fd);