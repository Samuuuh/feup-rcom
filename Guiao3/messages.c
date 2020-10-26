#include <stdio.h>
#include <unistd.h>

#include "messages.h"
#include "const_defines.h"
#include "state_machines.h"
#include "llfunctions.h"
#include "alarm.h"

extern int received_UA;
extern int received_RR;
extern int resent_times_write;
extern int Ns;

unsigned char SET[5] = {FLAG, A_Sender_Receiver, C_SET, BCC_SET, FLAG};
unsigned char UA[5] = {FLAG, A_Sender_Receiver, C_UA, BCC_UA, FLAG};
unsigned char DISC[5] = {FLAG, A_Sender_Receiver, C_DISC, BCC_DISC, FLAG};
unsigned char DATA[128];

enum current_state SET_state = start;
enum current_state UA_state = start;
enum current_state DISC_state = start;
enum current_state RR_state = start;

unsigned char calculateBCC2(unsigned char *message, int sizeMessage) {
  unsigned char BCC2 = message[4];
  for (int i = 5; i < sizeMessage; i++) {
    BCC2 ^= message[i];
  }

  return BCC2;
}

void write_SET(int fd) {
  int i = 0;
  while (i < 5) {
    write(fd, &SET[i], 1);
    i++;
  }

  printf("Sent: SET = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", SET[0], SET[1], SET[2], SET[3], SET[4]);
}

void read_SET(int fd) {
  unsigned char SET_read[5];
  int i = 0;

  SET_state = start;
  while (SET_state != stop) {
    read(fd, &SET_read[i], 1);

    i = process_SET(SET_read[i], &SET_state);
  }

  printf("Received: SET = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", SET_read[0], SET_read[1], SET_read[2], SET_read[3], SET_read[4]);
}

void write_UA(int fd) {
  int i = 0;
  while (i < 5) {
    write(fd, &UA[i], 1);
    i++;
  }
  
  printf("Sent: UA = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", UA[0], UA[1], UA[2], UA[3], UA[4]);
}

void read_UA(int fd) {
  unsigned char UA_read[5];
  int i = 0;

  UA_state = start;
  while (UA_state != stop) {
    read(fd, &UA_read[i], 1);

    i = process_UA(UA_read[i], &UA_state);
  }

  received_UA = TRUE;
  printf("Received: UA = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", UA_read[0], UA_read[1], UA_read[2], UA_read[3], UA_read[4]);
}

void write_DISC(int fd) {
  int i = 0;
  while (i < 5) {
    write(fd, &DISC[i], 1);
    i++;
  }
  
  printf("Sent: DISC = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", DISC[0], DISC[1], DISC[2], DISC[3], DISC[4]);
}

void read_DISC(int fd) {
  unsigned char DISC_read[5];
  int i = 0;

  DISC_state = start;
  while (DISC_state != stop) {
    read(fd, &DISC_read[i], 1);

    i = process_DISC(DISC_read[i], &DISC_state);
  }

  printf("Received: DISC = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", DISC_read[0], DISC_read[1], DISC_read[2], DISC_read[3], DISC_read[4]);
}

void write_RR(int fd) {
  unsigned char RR[5] = { FLAG, A_Sender_Receiver, C_RR(Ns), BCC_RR(Ns), FLAG };
  int i = 0;
  while (i < 5) {
    write(fd, &RR[i], 1);
    i++;
  }

  printf("Sent: RR = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n\n", RR[0], RR[1], RR[2], RR[3], RR[4]);
}

int read_RR(int fd) {
  unsigned char RR_read[5];
  int i = 0;

  received_RR = FALSE;
  resent_times_write = 0;

  alarm(3);

  RR_state = start;
  while (RR_state != stop) {
    read(fd, &RR_read[i], 1);

    i = process_RR(RR_read[i], &RR_state);
  }

  received_RR = TRUE;

  if (((RR_read[2] == C_REJ(0)) || (RR_read[2] == C_REJ(1))) && ((RR_read[3] == BCC_REJ(0)) || (RR_read[3] == BCC_REJ(1)))) {
    printf("Received: REJ = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n\n", RR_read[0], RR_read[1], RR_read[2], RR_read[3], RR_read[4]);
    return Ns;
  }

  printf("Received: RR = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n\n", RR_read[0], RR_read[1], RR_read[2], RR_read[3], RR_read[4]);

  return (RR_read[2] & 0x80 ? 1 : 0);
}

void write_REJ(int fd) {
  unsigned char REJ[5] = { FLAG, A_Sender_Receiver, C_REJ(Ns), BCC_REJ(Ns), FLAG };
  int i = 0;
  while (i < 5) {
    write(fd, &REJ[i], 1);
    i++;
  }

  printf("Sent: RR = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n\n", REJ[0], REJ[1], REJ[2], REJ[3], REJ[4]);
}