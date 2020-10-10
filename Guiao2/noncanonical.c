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

#define BCC_UA A_Receiver_Sender^C_UA

volatile int STOP=FALSE;

enum set_state {start, flag_rcv, a_rcv, c_rcv, bcc_ok, stop}

unsigned char UA[5] = {FLAG, A_Receiver_Sender, C_UA, BCC_UA, FLAG};

int process(enum set_state) {
  switch(set_state) {
    case start:
      if (SET[i] == FLAG) {
        set_state = flag_rcv;
        return 1;
      }
    case flag_rcv:
      if (SET[i] == FLAG) {
        set_state = flag_rcv;
      }
      else if (SET[i] == A_Receiver_Sender) {
        set_state = a_rcv;
        return 2;
      }
      else set_state = start;
      break;
    case a_rcv:
      if (SET[i] == FLAG) {
        set_state = flag_rcv;
      }
      else if (SET[i] == C_SET) {
        set_state = c_rcv;
        return 3;
      }
      else set_state = start;
      break;
    case c_rcv:
      if (SET[i] == FLAG) {
        set_state = flag_rcv;
      }
      else if (SET[i] == BCC_SET) {
        set_state = bcc_ok;
        return 4;
      }
      else set_state = start;
      break;
    case bcc_ok:
      if (SET[i] == FLAG) {
        set_state = stop;
      }
      else set_state = start;
    default:
      break;
  }
  return 0;
}

void read_SET(int fd) {
  unsigned char SET[5];
  
  while (set_state != stop) {
    read(fd, &SET[i], 1);

    i = process(set_state);
  }

  printf("Received: SET = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", SET[0], SET[1], SET[2], SET[3], SET[4]);
}

int main(int argc, char** argv)
{
  int fd,c, res;
  struct termios oldtio,newtio;
  char buf[255];

  if ( (argc < 2) || 
        ((strcmp("/dev/ttyS0", argv[1])!=0) && 
        (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
  fd = open(argv[1], O_RDWR | O_NOCTTY );
  if (fd <0) {perror(argv[1]); exit(-1); }

  if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 char received */


  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */


  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  read_SET(fd);

  write(fd, UA, 5);
  printf("Sent: UA = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", UA[0], UA[1], UA[2], UA[3], UA[4]);

  sleep(1);

  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
  return 0;
}
