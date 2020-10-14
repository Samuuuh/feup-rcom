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

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define BCC_UA A_Sender_Receiver^C_UA
#define BCC_SET A_Sender_Receiver^C_SET

volatile int STOP=FALSE;

enum set_state state = start;

unsigned char UA[5] = {FLAG, A_Sender_Receiver, C_UA, BCC_UA, FLAG};

int process(char received) {
  switch(state) {
    case start:
      if (received == FLAG) {
        state = flag_rcv;
        return 1;
      }
      break;
    case flag_rcv:
      if (received == FLAG) {
        state = flag_rcv;
        return 1;
      }
      else if (received == A_Sender_Receiver) {
        state = a_rcv;
        return 2;
      }
      else state = start;
      break;
    case a_rcv:
      if (received == FLAG) {
        state = flag_rcv;
        return 1;
      }
      else if (received == C_SET) {
        state = c_rcv;
        return 3;
      }
      else state = start;
      break;
    case c_rcv:
      if (received == FLAG) {
        state = flag_rcv;
        return 1;
      }
      else if (received == BCC_SET) {
        state = bcc_ok;
        return 4;
      }
      else state = start;
      break;
    case bcc_ok:
      if (received == FLAG) {
        state = stop;
      }
      else state = start;
    default:
      break;
  }
  return 0;
}

void read_SET(int fd) {
  unsigned char SET[5];
  
  int i = 0;

  while (state != stop) {
    read(fd, &SET[i], 1);

    i = process(SET[i]);
  }

  printf("Received: SET = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", SET[0], SET[1], SET[2], SET[3], SET[4]);
}

int main(int argc, char** argv)
{
  int fd,c, res;
  char buf[255];

  if ( (argc < 2) || 
        ((strcmp("/dev/ttyS0", argv[1])!=0) && 
        (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  struct applicationLayer application;

  llopen(&application);

  read_SET(application.fileDescriptor);

  write(application.fileDescriptor, UA, 5);
  printf("Sent: UA = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", UA[0], UA[1], UA[2], UA[3], UA[4]);

  sleep(1);

  tcsetattr(application.fileDescriptor,TCSANOW,&oldtio);
  close(fd);
  return 0;
}
