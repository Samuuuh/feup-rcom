#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include<arpa/inet.h>
#include <ctype.h>
#include <libgen.h>

#include "utils.h"

#define BUFFER_SIZE 1024

int parseArguments(char* argument, char* user, char* pass, char* host, char* file_path) {

  // Parse the initial part of the argument: "ftp://"
  if(strncmp(argument, "ftp://", 6) != 0) {
    fprintf(stderr,"Invalid Argument: it should start with 'ftp://'\n");
		return -1;
  }

  // Parse the part of the argument relative to the user
  int i = 6;
  while (argument[i] != ':') {
    user[i - 6] = argument[i];
    i++;
  }
  user[i - 6] = '\0';

  // Parse the part of the argument relative to the pass
  i++;
  int j = i;
  while (argument[j] != '@') {
    pass[j - i] = argument[j];
    j++;
  }
  pass[j - i] = '\0';

  // Parse the part of the argument relative to the host
  j++;
  int k = j;
  while (argument[k] != '/') {
    host[k - j] = argument[k];
    k++;
  }
  host[k - j] = '\0';

  // Parse the part of the argument relative to the url path
  k++;
  int l = k;
  while (argument[l] != '\0') {
    file_path[l - k] = argument[l];
    l++;
  }
  file_path[l - k] = '\0';

  return 0;
}

struct hostent* getIP(char *host) {
	struct hostent *h;

  /*
  struct hostent {
      char    *h_name;	Official name of the host. 
      char    **h_aliases;	A NULL-terminated array of alternate names for the host. 
      int     h_addrtype;	The type of address being returned; usually AF_INET.
      int     h_length;	The length of the address in bytes.
      char    **h_addr_list;	A zero-terminated array of network addresses for the host. 
      Host addresses are in Network Byte Order. 
  };

  #define h_addr h_addr_list[0]	The first address in h_addr_list. 
  */

  if ((h=gethostbyname(host)) == NULL) {  
    herror("gethostbyname");
    return NULL;
  }

  //printf("Host name  : %s\n", h->h_name);
  //printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));

  return h;
}

void readServerResponse(int sockfd, char *response) {
  enum code_state state = start;
  char character;

  while(state != code_received) {
    read(sockfd, &character, 1);
    //printf("Received %c\n", character);

    switch(state) {
      case start:
        if (isdigit(character)) {
          state = first_digit;
          response[0] = character;
        }
        break;
      case first_digit:
        if (isdigit(character)) {
          state = second_digit;
          response[1] = character;
        }
        else {
          state = start;
          memset(response,0,sizeof(response));
        }
        break;
      case second_digit:
        if (isdigit(character)) {
          state = third_digit;
          response[2] = character;
        }
        else {
          state = start;
          memset(response,0,sizeof(response));
        }
        break;
      case third_digit:
        if ((character == ' ') || (character == '-')) {
          state = code_received;
        }
        else {
          state = start;
          memset(response,0,sizeof(response));
        }
        break;
    }
  }
}

int login(int sockfd, char *user, char *pass) {
  char response[3];

  // Send username
	write(sockfd, "user ", 5);
  write(sockfd, user, strlen(user));
  write(sockfd, "\n", 1);

  readServerResponse(sockfd, response);

  if(strncmp(response, "331", 3) != 0) {
    fprintf(stderr,"Username not accepted\n");
		return -1;
  }

  // Send pass
	write(sockfd, "pass ", 5);
  write(sockfd, pass, strlen(pass));
  write(sockfd, "\n", 1);

  readServerResponse(sockfd, response);

  if(strncmp(response, "230", 3) != 0) {
    fprintf(stderr,"Password not accepted\n");
		return -2;
  }

  return 0;
}

int activatePassiveMode(int sockfd) {
  char response[3];
  char number[3];
  int i = 0;
  int pasv_numbers[6];
  int j = 0;

  write(sockfd, "pasv\n", 5);

  enum pasv_state state = pasv_start;
  char character;

  while(state != pasv_end) {
    read(sockfd, &character, 1);
    //printf("Received %c\n", character);

    switch(state) {
      case pasv_start:
        if (character == '(') {
          state = h1;
        }
        break;
      case h1:
        if (isdigit(character)) {
          number[i] = character;
          i++;
        }
        else if (character == ',') {
          state = h2;
          pasv_numbers[j] = atoi(number);
          memset(number,0,sizeof(number));
          i = 0;
          j++;
        }
        break;
      case h2:
        if (isdigit(character)) {
          number[i] = character;
          i++;
        }
        else if (character == ',') {
          state = h3;
          pasv_numbers[j] = atoi(number);
          memset(number,0,sizeof(number));
          i = 0;
          j++;
        }
        break;
      case h3:
        if (isdigit(character)) {
          number[i] = character;
          i++;
        }
        else if (character == ',') {
          state = h4;
          pasv_numbers[j] = atoi(number);
          memset(number,0,sizeof(number));
          i = 0;
          j++;
        }
        break;
      case h4:
        if (isdigit(character)) {
          number[i] = character;
          i++;
        }
        else if (character == ',') {
          state = p1;
          pasv_numbers[j] = atoi(number);
          memset(number,0,sizeof(number));
          i = 0;
          j++;
        }
        break;
      case p1:
        if (isdigit(character)) {
          number[i] = character;
          i++;
        }
        else if (character == ',') {
          state = p2;
          pasv_numbers[j] = atoi(number);
          memset(number,0,sizeof(number));
          i = 0;
          j++;
        }
        break;
      case p2:
        if (isdigit(character)) {
          number[i] = character;
          i++;
        }
        else if (character == ')') {
          state = pasv_end;
          pasv_numbers[j] = atoi(number);
        }
        break;
    }
  }

  int port = pasv_numbers[4]*256 + pasv_numbers[5];

  for (int i = 0 ; i < 6 ; i++) {
    printf("%d\n", pasv_numbers[i]);
  }

  return port;
}

int download_file(int sockfd, int sockfd_client, char* file_path) {
  // Send username
	write(sockfd, "retr ", 5);
  write(sockfd, file_path, strlen(file_path));
  write(sockfd, "\n", 1);

  char* filename;

  filename = basename(file_path);

  FILE *file = fopen(filename, "wb+");
  
  char file_part[BUFFER_SIZE];
  int bytes_read, elems_written;
  printf("FORA\n");

  while((bytes_read = read(sockfd_client, file_part, BUFFER_SIZE)) > 0) {
    printf("DENTRO\n");
    elems_written = fwrite(file_part, bytes_read, 1, file);
    printf("FILE_PART = %s\n", file_part);
    if (elems_written != 1) {
      fprintf(stderr,"Error downloading file\n");
		  return -1;
    }
  }

  fclose(file);

  return 0;
}