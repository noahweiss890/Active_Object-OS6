// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#define PORT 3490
#define BUFF_SIZE 1030

int sock;

void signal_handler_c(int sig) {
	if(sig == SIGINT) {
		printf("exiting\n");
		send(sock, "EXIT", sizeof("EXIT"), 0);
		close(sock);
		exit(0);
	}
}

int main(int argc, char const* argv[])
{
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

	signal(SIGINT, &signal_handler_c);

    char input[BUFF_SIZE];
    char text[BUFF_SIZE];
    while(1) {
		printf("INPUT: ");
        fgets(input, BUFF_SIZE, stdin);
		input[strlen(input) - 1] = 0;
		if(strcmp(input, "EXIT") == 0) {
			if(send(sock, "EXIT", sizeof("EXIT"), 0) < 0) {
            	perror("ERROR: sending input");
        	}
			break;
		}
        if(send(sock, input, BUFF_SIZE, 0) < 0) {
            perror("ERROR: sending input");
        }
        if(recv(sock, text, BUFF_SIZE, 0) < 0) {
            perror("ERROR: recieving");
        }
        printf("OUTPUT: %s\n", text);
    }

	close(sock);
	return 0;
}
