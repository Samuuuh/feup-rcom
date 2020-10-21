#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <string.h>

#include "llfunctions.h"
#include "const_defines.h"
#include "messages.h"
#include "alarm.h"
#include "state_machines.h"

struct termios oldtio,newtio;

int llopen(struct applicationLayer *application) {

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  if ( tcgetattr(application->fileDescriptor,&oldtio) == -1) { /* save current port settings */
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

  tcflush(application->fileDescriptor, TCIOFLUSH);

  if ( tcsetattr(application->fileDescriptor,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  if (application->status == TRANSMITTER) {
    signal(SIGALRM, alarm_handler);
    write_SET(application->fileDescriptor);
    alarm(3);
    read_UA(application->fileDescriptor);
    return 0;
  }
  else if (application->status == RECEIVER) {
    read_SET(application->fileDescriptor);
    write_UA(application->fileDescriptor);
    return 0;
  }

  // PERGUNTAR AO STOR O QUE É PARA METER AQUI 
  // RETURN VALUE 
  // 
  // PERGUNTAR
  //
  // PERGUNTAR
  //
  //
  // PERGUNTAR
  // FD?
  return 0;
}

int llwrite(struct applicationLayer *application, struct linkLayer *link) {
  int j = 0;
  char stuffed_msg[MAX_SIZE];

  // Calculate BCC2 with original Data
  unsigned char BCC2 = calculateBCC2(link->frame, link->baudRate);

  // Adds BCC2 to original Data, right before last FLAG
  char last_flag = link->frame[link->baudRate - 1];
  link->frame[link->baudRate - 1] = BCC2;
  link->frame[link->baudRate] = last_flag;
  link->baudRate++;

  // Adds initial FLAG, A, C, BCC1 bytes
  for (int i = 0; i < 4; i++) {
    stuffed_msg[j] = link->frame[i];
    j++;
  }

  // Byte stuffing in Data
  for (int i = 4; i < link->baudRate - 1; i++) {
    // 0x7e -> 0x7d 0x5e (FLAG byte)
    // 0x7d -> 0x7d 0x5d (ESC byte)
    if(link->frame[i] == FLAG) {
      stuffed_msg[j] = ESC;
      stuffed_msg[j + 1] = FLAG^0x20; // 0x5e
      j += 2;
    }
    else if (link->frame[i] == ESC) {
      stuffed_msg[j] = ESC;
      stuffed_msg[j + 1] = ESC^0x20; // 0x5d
      j += 2;
    }
    else {
      stuffed_msg[j] = link->frame[i];
      j++;
    }
  }

  // Adds last FLAG to msg
  stuffed_msg[j] = link->frame[link->baudRate - 1];
  j++;

  // Prints Data before stuffing (Original Message)
  for (int k = 0 ; k < link->baudRate ; k++) {
    printf("Sent: [%d] = 0x%02x\n", k, link->frame[k]);
  }

  strncpy(link->frame, stuffed_msg, sizeof(link->frame));

  // Write I-frame to the port
  int k = 0;
  while (k < j) {
    write(application->fileDescriptor, &link->frame[k], 1);
    k++;
  }

  return 0;
}

int llread(struct applicationLayer *application, struct linkLayer *link) {
  enum current_state DATA_state = start;
  int j = 0;
  unsigned char stuffed_msg[MAX_SIZE];

  int index = 0;
  while (DATA_state != stop) {
    read(application->fileDescriptor, &stuffed_msg[index], 1);

    index = process_DATA(stuffed_msg, index, &DATA_state);
  }

  // Adds initial FLAG, A, C, BCC1 bytes
  for (int i = 0; i < 4; i++) {
    link->frame[j] = stuffed_msg[i];
    j++;
  }

  // Check BCC1
  unsigned char BCC1 = (link->frame[1] ^ link->frame[2]);
  if (BCC1 != BCC_RR0_DATA) {
    printf("BCC1 ERROR\n");
    return -1;
  }

  for (int i = 4; i < index; i++) {
    // 0x7d 0x5e --> 0x7e (FLAG byte)
    // 0x7d 0x5d --> 0x7d (ESC byte)
    if(stuffed_msg[i] == ESC) {
      if (stuffed_msg[i + 1] == (FLAG ^ 0x20)) {  // 0x5e
        link->frame[j] = FLAG;
        i++;
        j++;
      }
      else if (stuffed_msg[i + 1] == (ESC ^ 0x20)) {  // 0x5d
        link->frame[j] = ESC;
        i++;
        j++;
      }
    }
    else {
      link->frame[j] = stuffed_msg[i];
      j++;
    }
  }

  // Print Data already destuffed (Original Message)
  for (int k = 0 ; k < j ; k++) {
    printf("Received: [%d] = 0x%02x\n", k, link->frame[k]);
  }

  unsigned char BCC2 = calculateBCC2(link->frame, j - 1);

  if (BCC2 != link->frame[j-2]) {
    printf("BCC2 ERROR\n");
  }

  return 0;
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