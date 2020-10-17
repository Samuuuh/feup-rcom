#include "state_machines.h"
#include "const_defines.h"

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