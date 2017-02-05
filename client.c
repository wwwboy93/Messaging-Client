#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define MAX_LEN 2048


void send_handler(int socketfd);// thread function that sends message to server
void rece_handler(int socketfd);// thread function that receives message
void Error(char* msg);// function called when error happens

int main(int argc, char **argv){
	/* establish connection with server*/
	if(argc<3){
		printf("Need the IP addr of server and port number\n");
		exit(0);
	}
	char* SERVER_ADDR = (char *)malloc(strlen(argv[1])+1);
	SERVER_ADDR[strlen(argv[1])] = '\0';
	int SERVER_PORT;
	
	/* Transfer the server addr and port number */
	strncpy(SERVER_ADDR, argv[1], strlen(argv[1]));
	
	SERVER_PORT = atoi(argv[2]);
	if(SERVER_PORT == 0) Error("Not valid port number");
	
	/* Try to connect with server*/
	int socketfd = -1;
	struct sockaddr_in server_address;
	struct hostent *server;
	struct in_addr ipv4addr;
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd < 0) 
		Error("Cannot create socket");
	
	inet_pton(AF_INET, SERVER_ADDR, &ipv4addr);
	server = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);

	if (server == NULL) {
		Error("Server not found");
	}
	/*Set the server info*/
	bzero((char *) &server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	bcopy((char *)server->h_addr,  (char *)&server_address.sin_addr.s_addr, server->h_length);
	server_address.sin_port = htons(SERVER_PORT);
	
	/*try to connect to server*/
	if (connect(socketfd,(struct sockaddr *) &server_address,sizeof(server_address)) < 0) 
		Error("Cannot connect to server");
	printf("connected to server\n");
	/* Create a thread send msg to server*/
	pthread_t send_thread;
	if( pthread_create( &send_thread , NULL ,  send_handler , socketfd) != 0)
		Error("Cannot Create thread"); 
	
	/* Create a thread receive msg from server*/	
	pthread_t rece_thread;
	if( pthread_create( &rece_thread , NULL ,  rece_handler , socketfd) != 0)
		Error("Cannot Create thread"); 

	pthread_join(rece_thread);
	pthread_join(send_thread);

	exit(0);
}
/* 	Thread: send_handler
*	Function: send the message to server, the first four bytes store
*	the length of this message, if send fails, exit.
*/
void send_handler(int socketfd){
	while(1){

		int n;
		char line[MAX_LEN];
		if(!fgets(line,MAX_LEN-1,stdin)){
			printf("message too long, please reinput");
			continue;
		}
		
		int send_len = strlen(line);
		if(strncmp(line, "\n" , 1) == 0) continue;
		/* use the first four bytes to indicate the length of msg*/
		char* buffer = (char *)malloc(sizeof(char)*(send_len + 5));
		buffer[send_len + 4] = '\0';
		memcpy(buffer,&send_len,4);

		/* send the message to the server*/
		memcpy(buffer+4, line, send_len);
		n = write(socketfd, buffer, send_len + 4);
		if(n < 0){
			free(buffer);
			pthread_exit(1); 
		}
		free(buffer);

	}
	pthread_exit(0);
}
/* 	Thread: rece_handler
*	Function: receive the message from server, the first four bytes store
*	the length of this message, if it reads with failure, exit.
*/
void rece_handler(int socketfd){
	while(1){
		int n;
		char rece_len[4];
		n = read(socketfd, rece_len, 4);
		if(n<0)	pthread_exit(1);
		if(n == 0) continue;
		int rece_length;// length of this msg
		memcpy(&rece_length, rece_len, 4);
		char* buffer = (char *)malloc(sizeof(char)*(rece_length+1));
		buffer[rece_length] = '\0';
		n = read(socketfd, buffer, rece_length);
		if(n < 0){
			free(buffer);
			pthread_exit(1);
		} 
		printf(" MSG: %s", buffer);
		free(buffer);
		
	}
	pthread_exit(0);
		
}
void Error(char* msg){
	fprintf(stderr, "error: %s\n", msg);
	exit(1);
}