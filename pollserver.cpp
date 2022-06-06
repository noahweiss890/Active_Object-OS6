#include <pthread.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include "reactor.hpp"

using namespace std;

#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 10	 // how many pending connections queue will hold
#define BUFF_SIZE 256

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

pReactor reactor;

void accept_func(void* void_fd);

void recv_func(void* void_fd) {
    int fd = *(int*)void_fd;
    char msg[BUFF_SIZE];
    int recv_bytes = recv(fd, msg, BUFF_SIZE, 0);
    if(recv_bytes < 0) {
        perror("recv");
    }
    else if(recv_bytes == 0 || strcmp(msg, "<EXIT>") == 0) {
        printf("fd %d left the chat room\n", fd);
        RemoveHandler(reactor, fd);
    }
    else {
        for(int i = 0; i < reactor->size; i++) {
            if(reactor->pfds[i].fd != fd && reactor->funcs[i] != accept_func) {
                if(send(reactor->pfds[i].fd, msg, BUFF_SIZE, 0) == -1) {
                    perror("send");
                }
            }
        }
    }
}

void accept_func(void* void_fd) {
    int fd = *(int*)void_fd;
    struct sockaddr_storage their_addr;
    char s[INET6_ADDRSTRLEN];
    socklen_t sin_size = sizeof their_addr;
    int new_fd = accept(fd, (struct sockaddr *)&their_addr, &sin_size);
    if(new_fd == -1) {
        perror("accept");
    }
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    printf("server: got connection from %s\n", s);

    InstallHandler(reactor, recv_func, new_fd);
}

void signal_handler_r(int sig) {
	if(sig == SIGINT) {
        pthread_cancel(reactor->tid);
		for(int i = 0; i < reactor->size; i++) {
			close(reactor->pfds[i].fd);
		}
		printf("\nfreeing stuff\n");
        free(reactor->pfds);
        free(reactor->funcs);
        free(reactor);
		exit(0);
	}
}

int main() {
    int sockfd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sigaction sa;
	int yes=1;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

    signal(SIGINT, &signal_handler_r);

	printf("server: waiting for connections...\n");

    reactor = (pReactor)newReactor();

    InstallHandler(reactor, accept_func, sockfd);
    
    pthread_join(reactor->tid, NULL);

    return 0;
}