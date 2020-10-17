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

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define BCC_SET A_Sender_Receiver^C_SET

volatile int STOP=FALSE;

int fd;

unsigned char UA[5];

int main(int argc, char** argv)
{
  struct termios oldtio,newtio;
  
  if ( (argc < 2) || 
        ((strcmp("/dev/ttyS0", argv[1])!=0) && 
        (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n"); 
    exit(1);
  }

  struct applicationLayer *application = malloc(sizeof(applicationLayer));

  application->fileDescriptor = open(argv[1], O_RDWR | O_NOCTTY );
  if (application->fileDescriptor < 0) {perror(argv[1]); exit(-1); }

  fd = application->fileDescriptor;

  application->status = TRANSMITTER;

  llopen(&application);

  sleep(1);
  
  if (tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
