// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>

#define PORT 3490
#define BUFF_SIZE 256

int sock;
pthread_t sending_tid, recieving_tid;

void signal_handler_cr(int sig) {
	if(sig == SIGINT) {
		printf("\nexiting\n");
		send(sock, "<EXIT>", BUFF_SIZE, 0);
		close(sock);
		exit(0);
	}
}

void* recieving_thread(void* temp) {
	char msg[BUFF_SIZE];
	while(1) {
		if(recv(sock, msg, BUFF_SIZE, 0) < 0) {
            perror("ERROR: recieving");
        }
        printf("\nRecieved message: %s\n", msg);
	}
}

void* sending_thread(void* temp) {
	char msg[BUFF_SIZE];
	while(1) {
		printf("\nEnter message: ");
        fgets(msg, BUFF_SIZE, stdin);
		msg[strlen(msg) - 1] = 0;
        if(send(sock, msg, BUFF_SIZE, 0) < 0) {
            perror("ERROR: sending input");
        }
	}
}

int main(int argc, char const* argv[])
{
	signal(SIGINT, &signal_handler_cr);

	sock = 0;
	struct sockaddr_in serv_addr;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr*)&serv_addr,
				sizeof(serv_addr))
		< 0) {
		printf("\nConnection Failed \n");
		return -1;
	}

	printf("To close the client press Ctrl+C\n");

    pthread_create(&sending_tid, NULL, sending_thread, NULL);
	pthread_create(&recieving_tid, NULL, recieving_thread, NULL);

	pthread_join(sending_tid, NULL);
	pthread_join(recieving_tid, NULL);
    
	close(sock);
	return 0;
}
