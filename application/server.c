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
#define BUFFER_SIZE 4096

static _Atomic unsigned int client_count = 0;
static int uid = 100;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
	char name[32];
	struct sockaddr_in address;
	int sockfd;
	int uid;
} client_t;

client_t *clients[MAX_CLIENTS];

void str_write() 
{
    printf(MAG"\r> "NRM);
    fflush(stdout);
}

void str_trim(char* arr, int length) 
{
    int i;
    for (i = 0; i < length; i++) 
    { 
        if (arr[i] == '\n') 
        {
            arr[i] = '\0';
            break;
        }
    }
}

void print_client_addr(struct sockaddr_in addr)
{
    printf("%d.%d.%d.%d",
        addr.sin_addr.s_addr & 0xff,
        (addr.sin_addr.s_addr & 0xff00) >> 8,
        (addr.sin_addr.s_addr & 0xff0000) >> 16,
        (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

void enqueue(client_t *client)
{
	pthread_mutex_lock(&mutex);

	for(int i=0; i < MAX_CLIENTS; ++i)
    {
		if(!clients[i])
        {
			clients[i] = client;
			break;
		}
	}

	pthread_mutex_unlock(&mutex);
}

void dequeue(int uid)
{
	pthread_mutex_lock(&mutex);

	for(int i = 0; i < MAX_CLIENTS; ++i)
    {
		if(clients[i])
        {
			if(clients[i]->uid == uid)
            {
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&mutex);
}

void send_message(char *s, int uid)
{
	pthread_mutex_lock(&mutex);

	for(int i=0; i<MAX_CLIENTS; ++i)
    {
		if(clients[i])
        {
			if(clients[i]->uid != uid){
				if(write(clients[i]->sockfd, s, strlen(s)) < 0){
					perror(RED"ERROR: write to file descriptor failed"NRM);
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&mutex);
}

void *handle_client(void *arg){
	char buffer[BUFFER_SIZE];
	char name[32];
	int leave = 0;

	client_count++;
	client_t *cli = (client_t *)arg;

	if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
		printf(RED"Have you entered the name? or check that the name should be greater that 2 char and less than 33 char.\n"NRM);
		leave = 1;
	}
	else{
		strcpy(cli->name, name);
		sprintf(buffer, BLU BLD"%s has joined\n"NRM, cli->name);
		printf("%s", buffer);
		send_message(buffer, cli->uid);
	}

	bzero(buffer, BUFFER_SIZE);

	while(1) {
		if (leave) {
			break;
		}

		int receive = recv(cli->sockfd, buffer, BUFFER_SIZE, 0);
		if (receive > 0){
			if(strlen(buffer) > 0){
				send_message(buffer, cli->uid);

				str_trim(buffer, strlen(buffer));
				printf(MAG" -> %s\n"NRM, buffer);
			}
		} 
		else if (receive == 0 || strcmp(buffer, "exit()") == 0){
			sprintf(buffer, RED BLD"%s has left\n"NRM, cli->name);
			printf("%s", buffer);
			send_message(buffer, cli->uid);
			leave = 1;
		} 
		else {
			printf(RED"ERROR: An error occurred while receiving message\n"NRM);
			leave = 1;
		}

		bzero(buffer, BUFFER_SIZE);
	}

	close(cli->sockfd);
	dequeue(cli->uid);
	free(cli);
	client_count--;
	pthread_detach(pthread_self());

	return NULL;
}

int main(int argc, char **argv){
	if(argc != 2){
		printf(MAG"Usage: %s <port>\n"NRM, argv[0]);
		return 1;
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);
	int option = 1;
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	pthread_t tid;

	/* Socket settings */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	serv_addr.sin_port = htons(port);

    /* Ignore pipe signals */
	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option)) < 0){
		perror(RED"ERROR: setsockopt failed"NRM);
    	return 0;
	}

	/* Bind */
	if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror(RED"ERROR: Socket binding failed"NRM);
		return 0;
	}

	/* Listen */
	if (listen(listenfd, 10) < 0) {
		perror(RED"ERROR: Socket listening failed"NRM);
		return 0;
	}

	printf(CYN BLD"======================== WELCOME TO THE CHATROOM ========================\n"NRM);

	while(1) {
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

		/* Check if max clients is reached */
		if(client_count == MAX_CLIENTS){
			printf(RED"Max clients reached. Rejected: "NRM);
			print_client_addr(cli_addr);
			printf(":%d\n", cli_addr.sin_port);
			close(connfd);
			continue;
		}

		/* Client settings */
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;

		/* Add client to the queue and fork thread */
		enqueue(cli);
		pthread_create(&tid, NULL, &handle_client, (void*)cli);

		/* Reduce CPU usage */
		sleep(1);
	}

	return 0;
}