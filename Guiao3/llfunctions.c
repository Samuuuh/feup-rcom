#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "llfunctions.h"
#include "const_defines.h"
#include "messages.h"
#include "alarm.h"
#include "state_machines.h"

struct termios oldtio,newtio;

int Ns = 0;

unsigned char frame_copy[MAX_SIZE * 2];
int length_copy;

int llopen(struct applicationLayer *application) {

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  application->fileDescriptor = open(application->port, O_RDWR | O_NOCTTY );
  if (application->fileDescriptor < 0) {perror(application->port); return -1; }

  if ( tcgetattr(application->fileDescriptor,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    return -1;
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

  tcflush(application->fileDescriptor, TCIOFLUSH);

  if ( tcsetattr(application->fileDescriptor,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    return -1;
  }

  printf("New termios structure set\n\n");

  if (application->status == TRANSMITTER) {
    fd_write = application->fileDescriptor;  // Used by alarm
    signal(SIGALRM, alarm_handler_open);
    write_SET(application->fileDescriptor);
    alarm(3);
    read_UA(application->fileDescriptor);
  }
  else if (application->status == RECEIVER) {
    read_SET(application->fileDescriptor);
    write_UA(application->fileDescriptor);
  }

  return application->fileDescriptor;
}

unsigned char calculateBCC2All(unsigned char *message, int sizeMessage) {
  unsigned char BCC2 = message[0];
  for (int i = 1; i < sizeMessage; i++) {
    BCC2 ^= message[i];
  }

  return BCC2;
}

int llwrite(int fd, unsigned char* buffer, int length) {
  int j = 0;
  unsigned char stuffed_msg[MAX_SIZE * 2];

  // Prints the Data Sent
  printf("Sent:\n");
  for (int k = 0 ; k < length; k++) {
    printf("DATA[%d] = 0x%02x\n", k, buffer[k]);
  }
  printf("\n");
	
  // Calculate BCC2 with Data
  unsigned char BCC2 = calculateBCC2All(buffer, length);

  // Byte stuffing in Data
  for (int i = 0; i < length; i++) {
    // 0x7e -> 0x7d 0x5e (FLAG byte)
    // 0x7d -> 0x7d 0x5d (ESC byte)
    if(buffer[i] == FLAG) {
      stuffed_msg[j] = ESC;
      stuffed_msg[j + 1] = 0x5e; // FLAG^0x20
      j += 2;
    }
    else if (buffer[i] == ESC) {
      stuffed_msg[j] = ESC;
      stuffed_msg[j + 1] = 0x5d; //  ESC^0x20
      j += 2;
    }
    else {
      stuffed_msg[j] = buffer[i];
      j++;
    }
  }
  
  // Byte stuffing BCC2
  if(BCC2 == FLAG) {
      stuffed_msg[j] = ESC;
      stuffed_msg[j + 1] = 0x5e; // FLAG^0x20
      j += 2;
   }
   else if (BCC2 == ESC) {
     stuffed_msg[j] = ESC;
     stuffed_msg[j + 1] = 0x5d; //  ESC^0x20
     j += 2;
   }
   else {
      stuffed_msg[j] = BCC2;
      j++;
   }

  int ind = 4, k = 0;
  sprintf(buffer, "%c%c%c%c", FLAG, A_Sender_Receiver, C_RR(Ns), BCC_RR(Ns));
  while (k < j) {
    buffer[ind] = stuffed_msg[k];
    ind++;
    k++;
  }

  // Adds BCC2 to original Data, right before last FLAG
  buffer[ind] = FLAG;
  ind++;

  length = ind;

  // Write I-frame to the port
  int b = 0;
  while (b < length + 6) {
    write(fd, &buffer[b], 1);
    b++;
  }

  // Make a frame copy, to resend in alarm, in case of trouble receiving RR
  for (int k = 0; k < length + 6; k++) {
    frame_copy[k] = buffer[k];
  }

  length_copy = length;

  // Receives RR or REJ answer
  int received_Ns = read_RR(fd);
  if (received_Ns != Ns) {  // Reader asked for next frame (received RR)
    Ns = received_Ns;
  }
  else {  // Reader asked for this frame again (received REJ) - Re-Write I-frame to the port
    int tries = 0;
    while (tries < 3) {   // ESTE VALOR VAI SER UM DEFINE (MAX_TRIES)
      int b = 0;
      while (b < length + 6) {
        write(fd, &buffer[b], 1);
        b++;
      }
      int received_Ns = read_RR(fd);
      if (received_Ns != Ns) {  // Reader asked for next frame (received RR)
        Ns = received_Ns;
        return length;
      }
      tries++;
    }
    return -1;
  }

  return length_copy;
}

int llread(int fd, unsigned char* buffer) {
  enum current_state DATA_state = start;
  int j = 0;
  unsigned char stuffed_msg[128];

  int index = 0;
  while (DATA_state != stop) {
    read(fd, &stuffed_msg[index], 1);

    index = process_DATA(stuffed_msg, index, &DATA_state);
  }

  // Adds initial FLAG, A, C, BCC1 bytes
  memset(buffer, 0, sizeof (buffer));
  for (int i = 0; i < 4; i++) {
    buffer[j] = stuffed_msg[i];
    j++;
  }
  
  // Check BCC1
  unsigned char BCC1 = (stuffed_msg[1] ^ stuffed_msg[2]);
  if (BCC1 != BCC_RR(Ns)) {
    printf("BCC1 ERROR\n");
    return -1;
  }

  for (int i = 4; i < index; i++) {
    // 0x7d 0x5e --> 0x7e (FLAG byte)
    // 0x7d 0x5d --> 0x7d (ESC byte)
    if(stuffed_msg[i] == ESC) {
      if (stuffed_msg[i + 1] == (FLAG ^ 0x20)) {  // 0x5e
        buffer[j] = FLAG;
        i++;
        j++;
      }
      else if (stuffed_msg[i + 1] == (ESC ^ 0x20)) {  // 0x5d
        buffer[j] = ESC;
        i++;
        j++;
      }
    }
    else {
      buffer[j] = stuffed_msg[i];
      j++;
    }
  }

  unsigned char BCC2 = calculateBCC2(buffer, j - 2);
	
  if (BCC2 != buffer[j-2]) {
    printf("BCC2 ERROR\n");
    write_REJ(fd);
    return -1;
  }
  
  // Return only the DATA, remove the special bytes
  unsigned char frame[128];
  for (int k = 0 ; k < j; k++) {
    frame[k] = buffer[k];
  }

  memset(buffer, 0, sizeof (buffer));

  for (int i = 0; i < j - 6; i++) {
    buffer[i] = frame[i + 4];
  }

  // Prints the Data received
  printf("Received:\n");
  for (int i = 0 ; i < j - 6; i++) {
    printf("DATA[%d] = 0x%02x\n", i, buffer[i]);
  }
  printf("\n");

  // Sends RR answer
  int received_Ns = (frame[2] & 0x80 ? 1 : 0);
  if (received_Ns == Ns) {  // Received the wanted frame (Ns requested)
    Ns = (frame[2] & 0x80 ? 0 : 1);
  }
  write_RR(fd);

  return j - 6;
}

int llclose(struct applicationLayer *application) {
  if (application->status == TRANSMITTER) {
    write_DISC(application->fileDescriptor);
    read_DISC(application->fileDescriptor);
    write_UA(application->fileDescriptor);
    return 0;
  }
  else if (application->status == RECEIVER) {
    read_DISC(application->fileDescriptor);
    write_DISC(application->fileDescriptor);
    read_UA(application->fileDescriptor);
    return 0;
  }
  return -1;
}
