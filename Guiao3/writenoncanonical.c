/*Non-Canonical Input Processing*/

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
       
#include "const_defines.h"
#include "llfunctions.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

volatile int STOP=FALSE;

extern struct termios oldtio;

int main(int argc, char** argv)
{ 
  if ((argc != 3) || 
        ((strcmp("/dev/ttyS0", argv[1])!=0) && 
        (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort input_file\n\tex: nserial /dev/ttyS1 pinguim.gif\n"); 
    exit(1);
  }

  struct applicationLayer application;

  application.status = TRANSMITTER;
  strncpy(application.port, argv[1], sizeof(application.port));

  if (llopen(&application) < 0) {
    printf("LLOPEN() failed\n");
    exit(2);
  }
  
  printf("LLOPEN() done successfully\n\n");


  // Read the Data Array from the file
  FILE *f;
  struct stat metadata;
  unsigned char *fileData;

  if ((f = fopen(argv[2], "rb")) == NULL) {
    perror("Error opening file!");
    exit(3);
  }
  stat(argv[2], &metadata);
  long int sizeFile = metadata.st_size;
  printf("This file has %ld bytes \n\n", sizeFile);

  fileData = (unsigned char *)malloc(sizeFile);

  fread(fileData, sizeof(unsigned char), sizeFile, f);

  // --------------------------------

  // Calculate the number of Data Packets to send
  int packet_number = sizeFile / MAX_SIZE;
  packet_number += ((sizeFile % MAX_SIZE) ? 1 : 0);

  // ----- Create Start Control Packet -----
  unsigned char start_packet[128];
  long int digits_V = log10l(sizeFile) + 1;
  start_packet[0] = 2;

  // Add file size info to Start Control Packet
  start_packet[1] = 0;
  start_packet[2] = digits_V;
  unsigned char V_string[64];
  sprintf(V_string, "%ld", sizeFile);
  for (int i = 0 ; i < digits_V; i++) {
    start_packet[i + 3] = (int) V_string[i];
  }

  // Add file name info to Start Control Packet
  start_packet[digits_V + 3] = 1;
  start_packet[digits_V + 4] = strlen(argv[2]);
  for (int j = 0; j < strlen(argv[2]); j++) {
    start_packet[digits_V + 5 + j] = argv[2][j];
  }

  int start_length = digits_V + 5 + strlen(argv[2]);
  // ------------------------------------------

  // Send Start Control Packet
  printf("-- Start Control Packet --\n");
  if (llwrite(application.fileDescriptor, start_packet, start_length) < 0) {
    printf("LLWRITE() failed\n");
    exit(4);
  }

  // Send Data Packets
  unsigned char data_packet[128];
  long int bytes_to_process = sizeFile;
  int data_ind = 0;
  for (int i = 1; i <= packet_number; i++) {
    memset(data_packet, 0, sizeof (data_packet));
    int K = MIN(MAX_SIZE, bytes_to_process);
    data_packet[0] = 1;
    data_packet[1] =  i % 255;
    data_packet[2] = K / 256;
    data_packet[3] =  K % 256;
    bytes_to_process -= K;
    for (int j = 0; j < K; j++) {
      data_packet[j + 4] = fileData[data_ind];
      data_ind++;
    }
    if (llwrite(application.fileDescriptor, data_packet, 4 + K) < 0) {
      printf("LLWRITE() failed\n");
      exit(4);
    }
  }

  // Create End Control Packet
  unsigned char end_packet[128];
  end_packet[0] = 3;
  end_packet[1] = 0;
  end_packet[2] = digits_V;
  digits_V = log10l(sizeFile) + 1;
  sprintf(V_string, "%ld", sizeFile);
  for (int i = 0 ; i < digits_V; i++) {
    end_packet[i + 3] = V_string[i];
  }

  // Send End Control Packet
  printf("-- End Control Packet --\n");
  if (llwrite(application.fileDescriptor, end_packet, 3 + digits_V) < 0) {
    printf("LLWRITE() failed\n");
    exit(4);
  }

  if (llclose(&application) < 0) {
    printf("LLCLOSE() failed\n");
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
