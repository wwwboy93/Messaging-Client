#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void Error(char* msg);

int client_socketfd[2];// store the sockets of clients
int turns[2];// keep track of which one is in use which is not
int connections;// Total number of connections
pthread_mutex_t lock;// mutex that provent concurrency of connections
void client_handler();// thread function that dealwith client


int main(int argc, char **argv){
	if(argc < 2){
		printf("Need port number\n");
		exit(0);
	}
	/* Convert the port number*/
	int port_listen;
	port_listen = atoi(argv[1]);
	if(port_listen == 0) Error("Not valid port number");

	/* Create socket and listen on the port*/
	int socketfd, client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	if((socketfd = socket(AF_INET, SOCK_STREAM, 0))<0)
		Error("Cannot create socket");
	bzero((char *) &server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port_listen);
	if (bind(socketfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) 
		Error("Cannot bind to port");

	listen(socketfd,2);
	if (pthread_mutex_init(&lock, NULL) != 0)
	    Error("mutex init fails");
	connections = 0;
	

	/* keep listening on the port, if the is already has two connections,
		stop accept new connection.
	*/
	while(1){
		if(connections == 2){
			sleep(1);// If we don't sleep, it will keep buging the CPU
			continue;
		}
		client_len= sizeof(client_address);

		int i = 0;
		while(i < 2){
			if(turns[i] == 0) break;
			i++;
		}

		client_socketfd[i] = accept(socketfd,(struct sockaddr *)&client_address, (socklen_t *)&client_len); 
		printf("accept a client\n");

		pthread_mutex_lock(&lock);	
		connections++;
		turns[i] = 1;
		pthread_mutex_unlock(&lock);

		pthread_t new;
		
		/* pass the client_socketfd as the parameter to the thread*/
		if( pthread_create( &new , NULL ,  client_handler , i) != 0)
		    Error("Cannot create thread");
		
	}
	
	exit(0);
}
/* 	Thread: client_handler
*	Function: receive the message from the client and resent it to 
*	another client if it exists. If any send or receive failure detected
*	close the socket and make connections decrement. The two threads manipulate
*	the connections together which could cause concurrency, so use mutex
*/
void client_handler(int no){
	int n;
	while(client_socketfd[no] > 0){
		char rece[4];// know how long the message is
		n = read(client_socketfd[no], rece, 4);
		if(n <= 0){
			close(client_socketfd[no]);
			pthread_mutex_lock(&lock);
			connections--;
			turns[no] = 0;
			pthread_mutex_unlock(&lock);
			break;
		}
		int rece_len;
		memcpy(&rece_len, rece, 4);
		char* buffer = (char *)malloc(rece_len+5);
		memcpy(buffer, rece, 4);
		/* Read the main part of the message*/ 
		n = read(client_socketfd[no], buffer+4, rece_len);
		if(n < 0){
			free(buffer);
			close(client_socketfd[no]);
			pthread_mutex_lock(&lock);
			connections--;
			turns[no] = 0;
			pthread_mutex_unlock(&lock);
			break;
		}
		/* If if has another client, send it to it*/
		if(connections == 2 && client_socketfd[(no+1)%2] > 0){
			
			n = write(client_socketfd[(no+1)%2], buffer, rece_len + 4);
			if(n < 0){
				close(client_socketfd[(no+1)%2]);
				pthread_mutex_lock(&lock);
				connections--;
				turns[(no+1)%2] = 0;
				pthread_mutex_unlock(&lock);
			}
		}
		free(buffer);
	}
	
	pthread_exit(0);
}


void Error(char* msg){
	fprintf(stderr, "error: %s\n", msg);
	exit(1);
}