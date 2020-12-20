/*      (C)2000 FEUP  */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <libgen.h>
#include <sys/time.h>

#include "utils.h"

#define SERVER_PORT 21
#define SERVER_ADDR "192.168.28.96"

#define MAX_SIZE 256

int main(int argc, char** argv){

	int	sockfd, sockfd_client;
	struct	sockaddr_in server_addr;
	struct	sockaddr_in server_addr_client;

	char user[MAX_SIZE];
	char pass[MAX_SIZE];
	char host[MAX_SIZE];
	char file_path[MAX_SIZE];

	struct hostent *h;

	if (argc != 2) {
		fprintf(stderr,"Usage: ./download ftp://[user]:[pass]@[host]/[url-path]\n");
		exit(1);
	}

	if (parseArguments(argv[1], user, pass, host, file_path) < 0) {
		fprintf(stderr,"Usage: ./download ftp://[user]:[pass]@[host]/[url-path]\n");
		exit(2);
	}

	if (user[0] == '\0' && pass[0] == '\0') {
		strcpy(user, "anonymous");
		strcpy(pass, "anypass");
	}

	printf("\n-------------------- INPUT DATA --------------------\n");
	printf("User: %s\nPassword: %s\nHost: %s\nURL path: %s\n", user, pass, host, file_path);

	if ((h = getIP(host)) == NULL) {
		fprintf(stderr,"Couldn't get Host IP\n");
		exit(3);
	}

	printf("\nHost name  : %s\n", h->h_name);
	printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));
	printf("----------------------------------------------------\n");
	
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(SERVER_PORT);		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket()");
		exit(4);
	}

	/*connect to the server*/
	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("connect()");
		exit(4);
	}

	char serverResponse[3];
	char fullResponse[1024];

	printf("\n>> Conecting to the server...\n");

	readServerResponse(sockfd, serverResponse, fullResponse);

	if (strncmp(serverResponse, "220", 3) != 0) {
		fprintf(stderr,"Connection lost\n");
		exit(5);
	}

	if (login(sockfd, user, pass) < 0) {
		fprintf(stderr,"Couldn't login\n");
		exit(6);
	}

	printf(">> Entering passive mode...\n\n");

	int port;
	if ((port = activatePassiveMode(sockfd)) < 0) {
		fprintf(stderr,"Couldn't enter passive mode\n");
		exit(7);
	}

	printf(">> Connecting to the client port...\n");

	/*server address handling*/
	bzero((char*)&server_addr_client,sizeof(server_addr_client));
	server_addr_client.sin_family = AF_INET;
	server_addr_client.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));	/*32 bit Internet address network byte ordered*/
	server_addr_client.sin_port = htons(port);		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((sockfd_client = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket()");
		exit(4);
	}

	/*connect to the server*/
	if(connect(sockfd_client, (struct sockaddr *)&server_addr_client, sizeof(server_addr_client)) < 0) {
		perror("connect()");
		exit(4);
	}

	printf("<< Client connection successful\n\n");

	printf(">> Starting downloading the file\n");

	struct timeval init_time;
	struct timeval current_time;
	gettimeofday(&init_time, 0);

	if (download_file(sockfd, sockfd_client, file_path) < 0) {
		fprintf(stderr,"Couldn't download file\n");
		exit(8);
	}

	gettimeofday(&current_time, 0);
	double elapsedTime = (current_time.tv_usec - init_time.tv_usec) / 1000.0 +
					(current_time.tv_sec - init_time.tv_sec) * 1000.0;

	char *filename = basename(file_path);

	printf("File %s downloaded successfully in %f seconds!\n", filename, elapsedTime/1000.0);

	if (close(sockfd_client) < 0) {
		perror("Error closing client socket");
		exit(9);
	}

	if (close(sockfd)) {
		perror("Error closing server socket");
		exit(9);
	}

	exit(0);
}