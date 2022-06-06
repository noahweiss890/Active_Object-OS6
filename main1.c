#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>
#include "ActiveObject.h"

#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 10	 // how many pending connections queue will hold
#define BUFF_SIZE 1030

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER; 

// Q3

int sock;

active_object* ao1;
active_object* ao2;
active_object* ao3;

void next_ao1(void* void_node) {
    pNode node = (pNode)void_node;
    enQ(ao2->que, node->data, node->fd);
}

void next_ao2(void* void_node) {
    pNode node = (pNode)void_node;
    enQ(ao3->que, node->data, node->fd);
}

void* send_to_client(void* void_node) {
    pNode node = (pNode)void_node;
    if(send(node->fd, node->data, BUFF_SIZE, 0) < 0) {
        perror("send error");
    }
    return void_node;
}

void do_nothing(void* data) { }

void* caesar_cipher(void* void_node) {
    pNode node = (pNode)void_node;
    char* word = (char*)node->data;
 
    for(int i = 0; i < strlen(word); i++) {
        if(isupper(word[i])) {
            word[i] = (word[i] + 1 - 65) % 26 + 65;
        }
        else {
            word[i] = (word[i] + 1 - 97) % 26 + 97;
        }
    }
    return node;
}

void* uppercase_lowercase(void* void_node) {
    pNode node = (pNode)void_node;
    char* word = (char*)node->data;
 
    for(int i = 0; i < strlen(word); i++) {
        if(isupper(word[i])) {
            word[i] = tolower(word[i]);
        }
        else {
            word[i] = toupper(word[i]);
        }
    }
    return node;
}

pthread_t tids[50];
int clients = 0;

void signal_handler_m(int sig) {
	if(sig == SIGINT) {
		printf("waiting for clients to terminate\n");
		for(int i = 0; i < clients; i++) {
			pthread_join(tids[i], NULL);
		}
		printf("destroying stuff\n");
		pthread_mutex_destroy(&lock1);
        pthread_mutex_destroy(&lock2);
        pthread_mutex_destroy(&lock3);
        pthread_cond_destroy(&cond1);
        pthread_cond_destroy(&cond2);
        pthread_cond_destroy(&cond3);
        destroyAO(ao1);
        destroyAO(ao2);
        destroyAO(ao3);
		exit(0);
	}
}

// Here is the function we made that will be run when a thread is created when someone connects to the server
void* client_thread(void* sock_fd) {
	int fd = *(int*)sock_fd;
	char msg[BUFF_SIZE];
    while(1) {
        if(recv(fd, msg, BUFF_SIZE, 0) < 0) {
            perror("recv error");
        }
        if(strcmp(msg, "EXIT") == 0) {
            printf("recieved EXIT!\n");
            close(fd); // close the file descripter for this client
			pthread_exit(NULL); // exit this thread
        }
        else {
            enQ(ao1->que, msg, fd);
        }
    }
	close(fd); // close the file descripter for this client
	pthread_exit(NULL); // exit this thread
}

int main() {

    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
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

    signal(SIGINT, &signal_handler_m);

    MyQueue* que1 = createQ(lock1, cond1);
    MyQueue* que2 = createQ(lock2, cond2);
    MyQueue* que3 = createQ(lock3, cond3);

    ao1 = newAO(que1, caesar_cipher, next_ao1);
    ao2 = newAO(que2, uppercase_lowercase, next_ao2);
    ao3 = newAO(que3, send_to_client, do_nothing);

	printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

        pthread_t ptid;
		pthread_create(&ptid, NULL, &client_thread, (void*)&new_fd); // creates a thread that runs the function "client_thread"
        tids[clients++] = ptid;
    }

    return 0;
}