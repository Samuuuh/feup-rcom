#include <stdio.h>
#include <unistd.h>

#include "messages.h"
#include "const_defines.h"
#include "state_machines.h"
#include "llfunctions.h"

unsigned char SET[5] = {FLAG, A_Sender_Receiver, C_SET, BCC_SET, FLAG};
unsigned char UA_Sender_Receiver[5] = {FLAG, A_Sender_Receiver, C_UA, BCC_UA_Sender_Receiver, FLAG};
unsigned char UA_Receiver_Sender[5] = {FLAG, A_Receiver_Sender, C_UA, BCC_UA_Receiver_Sender, FLAG};
unsigned char DISC_Sender_Receiver[5] = {FLAG, A_Sender_Receiver, C_DISC, BCC_DISC_Sender_Receiver, FLAG};
unsigned char DISC_Receiver_Sender[5] = {FLAG, A_Receiver_Sender, C_DISC, BCC_DISC_Receiver_Sender, FLAG};
unsigned char DATA[128];

enum current_state SET_state = start;
enum current_state UA_state = start;
enum current_state DISC_state = start;
enum current_state RR_state = start;

unsigned char calculateBCC2All(unsigned char *message, int sizeMessage) {
  unsigned char BCC2 = message[0];
  for (int i = 1; i < sizeMessage; i++) {
    BCC2 ^= message[i];
  }

  return BCC2;
}

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

void write_UA(struct applicationLayer app) {
  int i = 0;
  while (i < 5) {
    if(app.status == RECEIVER) {
      write(app.fileDescriptor, &UA_Sender_Receiver[i], 1);
    }
    else {
      write(app.fileDescriptor, &UA_Receiver_Sender[i], 1);
    }
      
    i++;
  }

  if(app.status == RECEIVER) {
    printf("Sent: UA = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", UA_Sender_Receiver[0], UA_Sender_Receiver[1], UA_Sender_Receiver[2], UA_Sender_Receiver[3], UA_Sender_Receiver[4]);
  }
  else printf("Sent: UA = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", UA_Receiver_Sender[0], UA_Receiver_Sender[1], UA_Receiver_Sender[2], UA_Receiver_Sender[3], UA_Receiver_Sender[4]);
  
}

int read_UA(int fd) {
  unsigned char UA_read[5];
  int i = 0;

  int res;
  UA_state = start;
  while (UA_state != stop) {
    if ((res = read(fd, &UA_read[i], 1)) == 0) {  // Didn't receive UA after timeout
      return FALSE;
    }

    i = process_UA(UA_read[i], &UA_state);
  }

  printf("Received: UA = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", UA_read[0], UA_read[1], UA_read[2], UA_read[3], UA_read[4]);
  return TRUE;
}

void write_DISC(struct applicationLayer app) {
  int i = 0;
  while (i < 5) {
    if(app.status == TRANSMITTER) {
      write(app.fileDescriptor, &DISC_Sender_Receiver[i], 1);
    }
    else {
      write(app.fileDescriptor, &DISC_Receiver_Sender[i], 1);
    }
    
    i++;
  }

  if(app.status == TRANSMITTER) {
    printf("Sent: DISC = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", DISC_Sender_Receiver[0], DISC_Sender_Receiver[1], DISC_Sender_Receiver[2], DISC_Sender_Receiver[3], DISC_Sender_Receiver[4]);
  }
  else {
    printf("Sent: DISC = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", DISC_Receiver_Sender[0], DISC_Receiver_Sender[1], DISC_Receiver_Sender[2], DISC_Receiver_Sender[3], DISC_Receiver_Sender[4]);
  }
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

void write_RR(int fd, int Ns) {
  unsigned char RR[5] = { FLAG, A_Sender_Receiver, C_RR(Ns), BCC_RR(Ns), FLAG };
  int i = 0;
  while (i < 5) {
    write(fd, &RR[i], 1);
    i++;
  }

  printf("Sent: RR = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n\n", RR[0], RR[1], RR[2], RR[3], RR[4]);
}

void write_REJ(int fd, int Ns) {
  unsigned char REJ[5] = { FLAG, A_Sender_Receiver, C_REJ(Ns), BCC_REJ(Ns), FLAG };
  int i = 0;
  while (i < 5) {
    write(fd, &REJ[i], 1);
    i++;
  }

  printf("Sent: REJ = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n\n", REJ[0], REJ[1], REJ[2], REJ[3], REJ[4]);
}

int read_RR(int fd, int* received_NS) {
  unsigned char RR_read[5];
  int i = 0, res = 0;

  RR_state = start;
  while (RR_state != stop) {
    if ((res = read(fd, &RR_read[i], 1)) == 0) {  // Didn't receive RR or REJ after timeout
      return FALSE;
    }

    i = process_RR_REJ(RR_read[i], &RR_state);
  }

  if (((RR_read[2] == C_REJ(0)) || (RR_read[2] == C_REJ(1))) && ((RR_read[3] == BCC_REJ(0)) || (RR_read[3] == BCC_REJ(1)))) {
    printf("Received: REJ = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n\n", RR_read[0], RR_read[1], RR_read[2], RR_read[3], RR_read[4]);
    return FALSE;
  }

  printf("Received: RR = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n\n", RR_read[0], RR_read[1], RR_read[2], RR_read[3], RR_read[4]);

  *received_NS = (RR_read[2] & 0x80 ? 1 : 0);

  return TRUE;
}