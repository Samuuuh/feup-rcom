#include "state_machines.h"
#include "const_defines.h"
#include "llfunctions.h"

extern int Ns;

int process_SET(char received, enum current_state *state) {
  switch(*state) {
    case start:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      break;
    case flag_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == A_Sender_Receiver) {
        *state = a_rcv;
        return 2;
      }
      else *state = start;
      break;
    case a_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == C_SET) {
        *state = c_rcv;
        return 3;
      }
      else *state = start;
      break;
    case c_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == BCC_SET) {
        *state = bcc_ok;
        return 4;
      }
      else *state = start;
      break;
    case bcc_ok:
      if (received == FLAG) {
        *state = stop;
      }
      else *state = start;
    default:
      break;
  }
  return 0;
}

int process_UA(char received, enum current_state *state) {
  switch(*state) {
    case start:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      break;
    case flag_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == A_Sender_Receiver) {
        *state = a_rcv;
        return 2;
      }
      else *state = start;
      break;
    case a_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == C_UA) {
        *state = c_rcv;
        return 3;
      }
      else *state = start;
      break;
    case c_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == BCC_UA) {
        *state = bcc_ok;
        return 4;
      }
      else *state = start;
      break;
    case bcc_ok:
      if (received == FLAG) {
        *state = stop;
      }
      else *state = start;
    default:
      break;
  }
  return 0;
}

int process_DISC(char received, enum current_state *state) {
  switch(*state) {
    case start:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      break;
    case flag_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == A_Sender_Receiver) {
        *state = a_rcv;
        return 2;
      }
      else *state = start;
      break;
    case a_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == C_DISC) {
        *state = c_rcv;
        return 3;
      }
      else *state = start;
      break;
    case c_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == BCC_DISC) {
        *state = bcc_ok;
        return 4;
      }
      else *state = start;
      break;
    case bcc_ok:
      if (received == FLAG) {
        *state = stop;
      }
      else *state = start;
    default:
      break;
  }
  return 0;
}

int process_DATA(char* message, int index, enum current_state *state) {
  unsigned char received = message[index];
  switch(*state) {
    case start:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      break;
    case flag_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == A_Sender_Receiver) {
        *state = a_rcv;
        return 2;
      }
      else *state = start;
      break;
    case a_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == C_RR(Ns)) { // alternar entre c rr 0 e 1
        *state = c_rcv;
        return 3;
      }
      else *state = start;
      break;
    case c_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == BCC_RR(Ns)) {
        *state = data_rcv;
        return 4;
      }
      else *state = start;
      break;
    case data_rcv:
      if (received == FLAG && message[index - 1] != ESC) {
        *state = stop;
      }
      index++;
      return index;
    default:
      break;
  }
  return 0;
}

int process_RR(char received, enum current_state *state) {
  switch(*state) {
    case start:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      break;
    case flag_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == A_Sender_Receiver) {
        *state = a_rcv;
        return 2;
      }
      else *state = start;
      break;
    case a_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == C_RR(Ns)) {
        *state = c_rcv;
        return 3;
      }
      else *state = start;
      break;
    case c_rcv:
      if (received == FLAG) {
        *state = flag_rcv;
        return 1;
      }
      else if (received == BCC_RR(Ns)) {
        *state = bcc_ok;
        return 4;
      }
      else *state = start;
      break;
    case bcc_ok:
      if (received == FLAG) {
        *state = stop;
      }
      else *state = start;
    default:
      break;
  }
  return 0;
}