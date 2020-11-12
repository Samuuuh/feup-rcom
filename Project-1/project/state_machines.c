#include "state_machines.h"
#include "const_defines.h"
#include "llfunctions.h"
#include <stdio.h>

extern int Ns_Enviado_Write;
extern int Ns_Recebido_Read;

int Ns = -1;

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
      else if (received == A_Sender_Receiver || received == A_Receiver_Sender) {
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
      else if (received == BCC_UA_Sender_Receiver || received == BCC_UA_Receiver_Sender) {
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
      else if (received == A_Sender_Receiver || received == A_Receiver_Sender) {
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
      else if (received == BCC_DISC_Sender_Receiver || received == BCC_DISC_Receiver_Sender) {
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
      else if (received == C_I0) {
        *state = c_rcv;
        Ns_Recebido_Read = 0;
        return 3;
      }
      else if (received == C_I1) {
        Ns_Recebido_Read = 1;
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
      else if ((received == BCC_C_I0) && (Ns_Recebido_Read == 0)) {
        *state = data_rcv;
        return 4;
      }
      else if ((received == BCC_C_I1) && (Ns_Recebido_Read == 1)) {
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

int process_RR_REJ(unsigned char received, enum current_state *state) {
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
      else if ((received == C_RR(0)) || (received == C_RR(1)) || (received == C_REJ(0)) || (received == C_REJ(1))) {
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
      else if ((received == BCC_RR(0)) || (received == BCC_RR(1)) || (received == BCC_REJ(0)) || (received == BCC_REJ(1))) {
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