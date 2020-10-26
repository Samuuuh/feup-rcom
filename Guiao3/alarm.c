#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "alarm.h"
#include "const_defines.h"
#include "messages.h"
#include "llfunctions.h"
#include "state_machines.h"

int received_UA = FALSE;
int received_RR = FALSE;
int resent_times_open = 0;
int resent_times_write = 0;

extern unsigned char frame_copy[MAX_SIZE * 2];
extern int length_copy;
extern int Ns;

void alarm_handler_open() {
  if (!received_UA) {
    write_SET(fd_write);
    resent_times_open++;
    if (resent_times_open < 3)
      alarm(3);
    else
      exit(1);
  }
}

void alarm_handler_write() {
  if (!received_RR) {
    // ESCREVER DE NOVO A TRAMA
    int b = 0;
    while (b < length_copy + 6) {
      write(fd_write, &frame_copy[b], 1);
      b++;
    }

    unsigned char RR_read[5];
    int i = 0;
    
    resent_times_write++;
    if (resent_times_write < 3)
      alarm(3);
    else
      exit(1);

    enum current_state RR_state = start;
    while (RR_state != stop) {
      read(fd_write, &RR_read[i], 1);

      i = process_RR(RR_read[i], &RR_state);
    }

    received_RR = TRUE;

    if (((RR_read[2] == C_REJ(0)) || (RR_read[2] == C_REJ(1))) && ((RR_read[3] == BCC_REJ(0)) || (RR_read[3] == BCC_REJ(1)))) {
      printf("Received: REJ = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n\n", RR_read[0], RR_read[1], RR_read[2], RR_read[3], RR_read[4]);
      return;
    }

    printf("Received: RR = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n\n", RR_read[0], RR_read[1], RR_read[2], RR_read[3], RR_read[4]);

    Ns = (RR_read[2] & 0x80 ? 1 : 0);
  }
}
