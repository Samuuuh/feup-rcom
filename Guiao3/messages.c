#include <stdio.h>
#include <unistd.h>

#include "messages.h"
#include "const_defines.h"
#include "state_machines.h"

#define BCC_SET A_Sender_Receiver^C_SET
#define BCC_UA A_Sender_Receiver^C_UA

unsigned char SET[5] = {FLAG, A_Sender_Receiver, C_SET, BCC_SET, FLAG};
unsigned char UA[5] = {FLAG, A_Sender_Receiver, C_UA, BCC_UA, FLAG};
unsigned char DISC[5] = {FLAG, A_Sender_Receiver, C_DISC, BCC_DISC, FLAG};

enum current_state SET_state = start;
enum current_state UA_state = start;
enum current_state DISC_state = start;

extern int received_UA;

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

  while (DISC_state != stop) {
    read(fd, &DISC_read[i], 1);

    i = process_DISC(DISC_read[i], &DISC_state);
  }

  printf("Received: DISC = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", DISC_read[0], DISC_read[1], DISC_read[2], DISC_read[3], DISC_read[4]);
}