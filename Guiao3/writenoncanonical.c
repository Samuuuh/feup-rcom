/*Non-Canonical Input Processing*/

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <signal.h>
       
#include "const_defines.h"
#include "llfunctions.h"
#include "alarm.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

volatile int STOP=FALSE;

extern struct termios oldtio;

extern int fd_write;

unsigned char UA[5];

int main(int argc, char** argv)
{ 
  if ( (argc < 2) || 
        ((strcmp("/dev/ttyS0", argv[1])!=0) && 
        (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n"); 
    exit(1);
  }

  struct applicationLayer application;

  application.status = TRANSMITTER;
  strncpy(application.port, argv[1], sizeof(application.port));

  if (llopen(&application) < 0) {
    printf("LLOPEN() failed");
    exit(2);
  }
  
  printf("LLOPEN() done successfully\n\n");

  FILE *f;
  struct stat metadata;// NÃO ESQUECER DE ALTERAR O A
  unsigned char *fileData;// NÃO ESQUECER DE ALTERAR O A
// NÃO ESQUECER DE ALTERAR O A
  if ((f = fopen("input_file.txt", "rb")) == NULL) {// NÃO ESQUECER DE ALTERAR O A
    perror("error opening file!");
    exit(-1);
  }
  stat("input_file.txt", &metadata);
  long int sizeFile = metadata.st_size;
  printf("This file has %ld bytes \n\n", sizeFile);

  fileData = (unsigned char *)malloc(sizeFile);

  fread(fileData, sizeof(unsigned char), sizeFile, f);

  // --------------------------------

  // Change alarm handler, for llwrite() function
  signal(SIGALRM, alarm_handler_write);

  // Calculate the number of Data Packets to send
  int packet_number = sizeFile / MAX_SIZE;
  packet_number += ((sizeFile % MAX_SIZE) ? 1 : 0);

  // Create Start Control Packet
  unsigned char start_packet[128];
  long int digits_V = log10l(sizeFile) + 1;
  sprintf(start_packet, "%d%d%ld", 2, 0, digits_V);
  unsigned char V_string[64];
  sprintf(V_string, "%ld", sizeFile);
  for (int i = 0 ; i < digits_V; i++) {
    start_packet[i + 3] = V_string[i];
  }

  // Send Start Control Packet
  printf("-- Start Control Packet --\n");
  if (llwrite(application.fileDescriptor, start_packet, 3 + digits_V) < 0) {
    printf("LLWRITE() failed");
    exit(4);
  }

  // Send Data Packets
  unsigned char data_packet[128];
  long int bytes_to_process = sizeFile;
  int data_ind = 0;
  for (int i = 1; i <= packet_number; i++) {
    memset(data_packet, 0, sizeof (data_packet));
    int K = MIN(MAX_SIZE, bytes_to_process);
    sprintf(data_packet, "%d%c%d%d", 1, i % 255, K / 256, K % 256);   // C, N, L2, L1
    bytes_to_process -= K;
    for (int j = 0; j < K; j++) {
      data_packet[j + 4] = fileData[data_ind];
      data_ind++;
    }
    if (llwrite(application.fileDescriptor, data_packet, 4 + K) < 0) {
      printf("LLWRITE() failed");
      exit(4);
    }
  }

  // Create End Control Packet
  unsigned char end_packet[128];
  sprintf(end_packet, "%d%d%ld", 3, 0, digits_V);
  digits_V = log10l(sizeFile) + 1;
  sprintf(V_string, "%ld", sizeFile);
  for (int i = 0 ; i < digits_V; i++) {
    end_packet[i + 3] = V_string[i];
  }

  printf("-- End Control Packet --\n");
  // Send End Control Packet
  if (llwrite(application.fileDescriptor, end_packet, 3 + digits_V) < 0) {
    printf("LLWRITE() failed");
    exit(4);
  }

  if (llclose(&application) < 0) {
    printf("LLCLOSE() failed");
    exit(5);
  }

  printf("LLCLOSE() done successfully\n");

  sleep(1);
  
  if (tcsetattr(application.fileDescriptor,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(application.fileDescriptor);
  return 0;
}
