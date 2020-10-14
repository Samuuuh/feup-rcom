#include "llfunctions.h"
#include "const_defines.h"

struct termios oldtio,newtio;

int llopen(struct applicationLayer *application) {

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
    
  *application.fileDescriptor = open(argv[1], O_RDWR | O_NOCTTY );
  if (*application.fileDescriptor < 0) {perror(argv[1]); exit(-1); }

  if ( tcgetattr(*application.fileDescriptor,&oldtio) == -1) { /* save current port settings */
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
    leitura do(s) próximo(s) caracter(es)
  */

  tcflush(*application.fileDescriptor, TCIOFLUSH);

  if ( tcsetattr(*application.fileDescriptor,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");
}

int llwrite(int fd, char * buffer, int length) {

}

int llread(int fd, char * buffer) {

}

int llclose(int fd) {
    
}