#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define NRM "\x1B[0m"
#define BLD "\033[1m"

#define MAX_CLIENTS 25
#define BUFFER_SZ 4096
#define LENGTH 64

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

void str_write() {
	printf(MAG"> "NRM);
	fflush(stdout);
}

void str_trim(char* arr, int length) {
	int i;
	for (i = 0; i < length; i++) {
		if (arr[i] == '\n') {
			arr[i] = '\0';
			break;
		}
	}
}

void catch_ctrl_c() {
    flag = 1;
}

void send_msg() {
	char message[LENGTH] = {};
	char buffer[LENGTH + 64] = {};

	while(1) {
		str_write();
		fgets(message, LENGTH, stdin);
		str_trim(message, LENGTH);

		if (strcmp(message, "exit()") == 0) {
			break;
		}
		else {
			sprintf(buffer, GRN BLD"%s: "NRM "%s""\n", name, message);
			send(sockfd, buffer, strlen(buffer), 0);
		}

		bzero(message, LENGTH);
		bzero(buffer, LENGTH + 64);
	}
	catch_ctrl_c();
}

void recv_msg() {
	char message[LENGTH] = {};
	while (1) {
		int receive = recv(sockfd, message, LENGTH, 0);
		if (receive > 0) {
			printf("%s", message);
			str_write();
		} 
		else {
			break;
		} 
		memset(message, 0, sizeof(message));
	}
}

int main(int argc, char **argv){
	if(argc != 2){
		printf(MAG"Usage: %s <port>\n"NRM, argv[0]);
		return 1;
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);

	signal(SIGINT, catch_ctrl_c);

	printf("Please enter your name: ");
  	fgets(name, 32, stdin);
  	str_trim(name, strlen(name));


	if (strlen(name) > 32 || strlen(name) < 2){
		printf(RED"Name must be less than 33 and more than 2 characters.\n"NRM);
		return 1;
	}

	struct sockaddr_in server_addr;

	/* Socket settings */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
  	server_addr.sin_family = AF_INET;
  	server_addr.sin_addr.s_addr = inet_addr(ip);
  	server_addr.sin_port = htons(port);


	// Connect to Server
	int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err == -1) {
		printf(RED"ERROR: Not able to connect\n"NRM);
		return 1;
	}

	// Send name
	send(sockfd, name, 32, 0);

	printf(CYN BLD"======================== WELCOME TO THE CHATROOM ========================\n"NRM);

	pthread_t send_msg_thread;
	if(pthread_create(&send_msg_thread, NULL, (void *) send_msg, NULL) != 0){
		printf(RED"ERROR: error in creating pthread\n"NRM);
		return 1;
	}

	pthread_t recv_msg_thread;
	if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg, NULL) != 0){
		printf(RED"ERROR: error in creating pthread\n"NRM);
		return 1;
	}

	while (1){
		if(flag){
			printf(YEL"\nBye, see you again!\n"NRM);
			break;
    	}
	}
	close(sockfd);

	return 0;
}