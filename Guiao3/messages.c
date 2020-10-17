#include "messages.h"
#include "state_machines.h"

unsigned char SET[5] = {FLAG, A_Sender_Receiver, C_SET, BCC_SET, FLAG};
unsigned char UA[5] = {FLAG, A_Sender_Receiver, C_UA, BCC_UA, FLAG};

enum current_state transmitter_state = start;
enum current_state receiver_state = start;

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

  while (receiver_state != stop) {
    read(fd, &SET_read[i], 1);

    i = process_SET(SET_read[i]);
  }

  printf("Received: SET = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", SET_read[0], SET_read[1], SET_read[2], SET_read[3], SET_read[4]);
}

void write_UA(int fd) {
  write(application.fileDescriptor, UA, 5);
  printf("Sent: UA = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", UA[0], UA[1], UA[2], UA[3], UA[4]);
}

void read_UA(int fd) {
  unsigned char UA_read[5];
  int i = 0;

  while (transmitter_state != stop) {
    read(fd, &UA_read[i], 1);

    i = process_UA(UA_read[i]);
  }

  received_UA = TRUE;
  printf("Received: UA = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", UA_read[0], UA_read[1], UA_read[2], UA_read[3], UA_read[4]);
}