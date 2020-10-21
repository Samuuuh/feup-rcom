/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "const_defines.h"
#include "llfunctions.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define BCC_SET A_Sender_Receiver^C_SET

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

  application.fileDescriptor = open(argv[1], O_RDWR | O_NOCTTY );
  if (application.fileDescriptor < 0) {perror(argv[1]); exit(-1); }

  fd_write = application.fileDescriptor;

  application.status = TRANSMITTER;

  if (llopen(&application) < 0) {
    perror("LLOPEN() failed");
    exit(2);
  }

  struct linkLayer link;

  unsigned char data[] = { FLAG, A_Sender_Receiver, C_RR_0, BCC_RR0_DATA, 0x55, 0x84, FLAG, 0x44, 0x55, ESC, 0x77, 0x55, FLAG };
  strncpy(link.frame, data, sizeof(link.frame));
  link.baudRate = strlen(link.frame);
  link.numTransmissions = 3;
  strncpy(link.port, argv[1], sizeof(link.port));
  link.sequenceNumber = 0;
  link.timeout = 3;

  llwrite(&application, &link);

  if (llclose(&application) < 0) {
    perror("LLCLOSE() failed");
    exit(3);
  }

  sleep(1);
  
  if (tcsetattr(application.fileDescriptor,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(application.fileDescriptor);
  return 0;
}