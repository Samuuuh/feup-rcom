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

#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"

#define MAX_SIZE 256

int main(int argc, char** argv){

	int	sockfd;
	struct	sockaddr_in server_addr;
	char	buf[] = "Mensagem de teste na travessia da pilha TCP/IP\n";
	int	bytes;

	char user[MAX_SIZE];
	char pass[MAX_SIZE];
	char host[MAX_SIZE];
	char url_path[MAX_SIZE];

	if (argc != 2) {
		fprintf(stderr,"Usage: ./download ftp://[user]:[pass]@[host]/[url-path]\n");
		exit(1);
	}

	if (parseArguments(argv[1], user, pass, host, url_path) < 0) {
		fprintf(stderr,"Usage: ./download ftp://[user]:[pass]@[host]/[url-path]\n");
		exit(2);
	}

	printf("User: |%s|\nPass: |%s|\nHost: |%s|\nURL path: |%s|\n", user, pass, host, url_path);

	return 0;		// Testar parseArguments - Apagar no futuro
	
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(SERVER_PORT);		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket()");
		exit(3);
	}

	/*connect to the server*/
	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("connect()");
		exit(4);
	}

  /*send a string to the server*/
	bytes = write(sockfd, buf, strlen(buf));
	printf("Bytes escritos %d\n", bytes);

	close(sockfd);
	exit(0);
}