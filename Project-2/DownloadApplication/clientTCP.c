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

#include "utils.h"

#define SERVER_PORT 21
#define SERVER_ADDR "192.168.28.96"

#define MAX_SIZE 256

int main(int argc, char** argv){

	int	sockfd, sockfd_client;
	struct	sockaddr_in server_addr;
	struct	sockaddr_in server_addr_client;
	char	buf[] = "Mensagem de teste na travessia da pilha TCP/IP\n";
	int	bytes;

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

	printf("User: |%s|\nPass: |%s|\nHost: |%s|\nURL path: |%s|\n", user, pass, host, file_path);

	if ((h = getIP(host)) == NULL) {
		fprintf(stderr,"Couldn't get Host IP\n");
		exit(3);
	}

	printf("Host name  : %s\n", h->h_name);
  printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));
	
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

	readServerResponse(sockfd, serverResponse, fullResponse);

	if (strncmp(serverResponse, "220", 3) != 0) {
		fprintf(stderr,"Connection lost\n");
		exit(5);
	}

	if (login(sockfd, user, pass) < 0) {
		fprintf(stderr,"Couldn't login\n");
		exit(6);
	}

	printf("User logged in\n");

	int port;

	if ((port = activatePassiveMode(sockfd)) < 0) {
		fprintf(stderr,"Couldn't enter passive mode\n");
		exit(7);
	}

	printf("PORT: %d\n", port);

	/*server address handling*/
	bzero((char*)&server_addr_client,sizeof(server_addr_client));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */
    
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

	if (download_file(sockfd, sockfd_client, file_path) < 0) {
		fprintf(stderr,"Couldn't download file\n");
		exit(8);
	}

	printf("File downloaded successfully!\n");

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