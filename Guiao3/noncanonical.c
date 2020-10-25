/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "const_defines.h"
#include "llfunctions.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */

volatile int STOP=FALSE;

extern struct termios oldtio;

int main(int argc, char** argv)
{
  int c, res;
  char buf[255];

  if ( (argc < 2) || 
        ((strcmp("/dev/ttyS0", argv[1])!=0) && 
        (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  struct applicationLayer application;

  application.status = RECEIVER;
  strncpy(application.port, argv[1], sizeof(application.port));

  if (llopen(&application) < 0) {
    printf("LLOPEN() failed");
    exit(2);
  }

  printf("LLOPEN() done successfully\n\n");

  // Read Control Start Packet
  unsigned char start_packet[128];
  if ((res = llread(application.fileDescriptor, start_packet)) < 0) {
    printf("LLREAD() failed");
    exit(3);
  }

  // Check if the packet received was the Control Start Packet (C = 2)
  if (((long int) start_packet[0] - 48) != 2) {
    printf("Received a wrong Start Packet (C != 2)");
    exit(4);
  }

  int file_size = 10;   // MUDAR ISTO, PROCESSA START PACKET PARA SABER ISTO
  unsigned char total_data[file_size];
   
  // Calculate the number of Data Packets to receive
  int packet_number = file_size / MAX_SIZE;
  packet_number += ((file_size % MAX_SIZE) ? 1 : 0);

  // Read Data Packets
  unsigned char data_packet[128];
  for (int i = 0; i < packet_number; i++) {
    if ((res = llread(application.fileDescriptor, data_packet)) < 0) {
      printf("LLREAD() failed");
      exit(3);
    }
  }

  // Read Control End Packet
  if ((res = llread(application.fileDescriptor, start_packet)) < 0) {
    printf("LLREAD() failed");
    exit(3);
  }

  // Check if the packet received was the Control End Packet (C = 3)
  if (((long int) start_packet[0] - 48) != 3) {
    printf("Received a wrong End Packet (C != 3)");
    exit(4);
  }

  if (llclose(&application) < 0) {
    printf("LLCLOSE() failed");
    exit(5);
  }

  printf("LLCLOSE() done successfully\n");

  sleep(1);

  tcsetattr(application.fileDescriptor,TCSANOW,&oldtio);
  close(application.fileDescriptor);
  return 0;
}
