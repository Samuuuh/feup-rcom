/*Non-Canonical Input Processing*/

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "const_defines.h"
#include "llfunctions.h"

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

  printf("fd_write = %d\n", fd_write);

  struct linkLayer link;

  unsigned char data[] = { FLAG, A_Sender_Receiver, C_RR_0, BCC_RR0_DATA, 0x55, 0x84, FLAG, 0x44, 0x55, ESC, 0x77, 0x55, FLAG };
  strncpy(link.frame, data, sizeof(link.frame));
  link.baudRate = strlen(link.frame);
  link.numTransmissions = 3;
  strncpy(link.port, argv[1], sizeof(link.port));
  link.sequenceNumber = 0;
  link.timeout = 3;

  if (llwrite(&application, &link) < 0) {
    printf("LLWRITE() failed");
    exit(3);
  }

  if (llclose(&application) < 0) {
    printf("LLCLOSE() failed");
    exit(4);
  }

  sleep(1);
  
  if (tcsetattr(application.fileDescriptor,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(application.fileDescriptor);
  return 0;
}