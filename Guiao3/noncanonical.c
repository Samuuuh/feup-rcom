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

  if ( (argc != 3) || 
        ((strcmp("/dev/ttyS0", argv[1])!=0) && 
        (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort output_file\n\tex: nserial /dev/ttyS1 pinguim.gif\n");
    exit(1);
  }

  struct applicationLayer application;

  application.status = RECEIVER;
  strncpy(application.port, argv[1], sizeof(application.port));

  if (llopen(&application) < 0) {
    printf("LLOPEN() failed\n");
    exit(2);
  }

  printf("LLOPEN() done successfully\n\n");

  // Read Control Start Packet
  printf("-- Start Control Packet --\n");
  unsigned char start_packet[128];
  if ((res = llread(application.fileDescriptor, start_packet)) < 0) {
    printf("LLREAD() failed\n");
    exit(3);
  }

  // Check if the packet received was the Control Start Packet (C = 2)
  if (((long int) start_packet[0] - 48) != 2) {
    printf("Received a wrong Start Packet (C != 2)");
    exit(4);
  }

  // Check if type of first parameter is the file size (T = 0)
  if (((long int) start_packet[1] - 48) != 0) {
    printf("First Parameter is not File Size. (T != 0)");
    exit(4);
  }

  FILE *file = fopen(argv[2], "wb+");

  // Read Data Packets
  unsigned char data_packet[128];
  while(TRUE) {
    if ((res = llread(application.fileDescriptor, data_packet)) < 0) {
      printf("LLREAD() failed\n");
      exit(3);
    }
    if (((long int) data_packet[0] - 48) == 3)   // Received Control End Packet
      break;
    // Mandar para o ficheiro
    int K = 256 * (data_packet[2] - 48) + (data_packet[3] - 48);
    for (int i = 0 ; i < K; i++) {
      fwrite((void *)&data_packet[i + 4], 1, 1, file);
    }
  }

  fclose(file);

  if (llclose(&application) < 0) {
    printf("LLCLOSE() failed\n");
    exit(5);
  }

  printf("LLCLOSE() done successfully\n");

  sleep(1);

  tcsetattr(application.fileDescriptor,TCSANOW,&oldtio);
  close(application.fileDescriptor);
  return 0;
}
