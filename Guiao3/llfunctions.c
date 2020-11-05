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
#include "state_machines.h"

struct termios oldtio,newtio;

int erro = 0;
int Ns_Enviado_Write = 0;
int Ns_Recebido_Read = 0;
int t = 0;

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

  newtio.c_cc[VTIME]    = TIMEOUT * 10;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 char received */

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
    write_SET(application->fileDescriptor);
    int resent_times_open = 0;
    while (resent_times_open < 3) {
      if (!read_UA(application->fileDescriptor)) {
        write_SET(application->fileDescriptor);
        resent_times_open++;
      }
      else
        break;
    }
    if (resent_times_open >= 3)
      return -1;
  }
  else if (application->status == RECEIVER) {
    read_SET(application->fileDescriptor);
    write_UA(*application);
  }

  return application->fileDescriptor;
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
    if (buffer[i] == FLAG) {
      stuffed_msg[j] = ESC;
      stuffed_msg[j + 1] = 0x5e; // FLAG^0x20
      j += 2;
    }
    else if (buffer[i] == ESC) {
      stuffed_msg[j] = ESC;
      stuffed_msg[j + 1] = 0x5d; // ESC^0x20
      j += 2;
    }
    else {
      stuffed_msg[j] = buffer[i];
      j++;
    }
  }
  
  // Byte stuffing of BCC2
  if (BCC2 == FLAG) {
    stuffed_msg[j] = ESC;
    stuffed_msg[j + 1] = (FLAG ^ 0x20); // 0x5e
    j += 2;
  }
  else if (BCC2 == ESC) {
    stuffed_msg[j] = ESC;
    stuffed_msg[j + 1] = (ESC ^ 0x20); // 0x5d
    j += 2;
  }
  else {
    stuffed_msg[j] = BCC2;
    j++;
  }

  // Adds the first 4 special bytes (F, A, C, BCC1)
  int ind = 4, k = 0;

  if(Ns_Enviado_Write == 0) {
    sprintf(buffer, "%c%c%c%c", FLAG, A_Sender_Receiver, C_I0, BCC_C_I0);
  }
  else {
    sprintf(buffer, "%c%c%c%c", FLAG, A_Sender_Receiver, C_I1, BCC_C_I1);
  }

  
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
  t++;
  // Receives RR or REJ answer
  int received_NS;
  int received_RR = read_RR(fd, &received_NS);
  printf("received: %d sent %d \n", received_NS, Ns_Enviado_Write);
  if ((!received_RR) || (received_NS == Ns_Enviado_Write)) { // old if (!received_RR) add || (t == 500) to simulate error || (t == 500)
    int resent_times_write = 0;
    while (resent_times_write < 3) {
      printf("erro");
      printf("Resending Frame...\n\n");
      // Re-Write I-frame to the port
      int b = 0;
      while (b < length + 6) {
        write(fd, &buffer[b], 1);
        b++;
      }

      if ((!read_RR(fd, &received_NS)) || (received_NS == Ns_Enviado_Write)) {// old if ((!read_RR(fd, &received_NS)))
        resent_times_write++;
      }
      else
        break;
    }
    if (resent_times_write >= 3)
      return -1;
  }

  printf("2 received ns %d sent %d\n", received_NS, Ns_Enviado_Write);

  // Se o valor for diferente, alterar o Ns.
  if (received_NS != Ns_Enviado_Write) {  // Reader asked for next frame (received RR), change Ns
    Ns_Enviado_Write = received_NS;
  }
  else {
    // Reenviar outra vez até receber o prox Ns?? feito em cima 
  }

  return length;
}

int llread(int fd, unsigned char* buffer) {
  erro++;

  enum current_state DATA_state = start;
  int j = 0;
  unsigned char stuffed_msg[128];

  // Reads the Packet Received
  int index = 0;
  while (DATA_state != stop) {
    read(fd, &stuffed_msg[index], 1);

    index = process_DATA(stuffed_msg, index, &DATA_state);

    printf("\n");
  }

  // Adds initial FLAG, A, C, BCC1 bytes
  memset(buffer, 0, sizeof (buffer));
  for (int i = 0; i < 4; i++) {
    buffer[j] = stuffed_msg[i];
    j++;
  }
  
  // Check BCC1
  unsigned char BCC1 = (stuffed_msg[1] ^ stuffed_msg[2]);
  if(Ns_Recebido_Read == 0) {
    if (BCC1 != BCC_C_I0) {
      printf("BCC1 ERROR\n");
      return -1;
    }
  }
  else {
    if (BCC1 != BCC_C_I1) {
      printf("BCC1 ERROR\n");
      return -1;
    }
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

  // Return only the DATA, remove the special bytes
  unsigned char frame[128];
  for (int k = 0 ; k < j; k++) {
    frame[k] = buffer[k];
  }

  // Sends RR answer
  unsigned char BCC2 = calculateBCC2(buffer, j - 2);
	
  if (BCC2 != buffer[j-2]) {
    printf("BCC2 ERROR. Asking Emissor to resend the packet...\n");
    printf("BCC2 = 0x%02x\n", BCC2);
    printf("buffer[j-2] = 0x%02x\n", buffer[j-2]);
    // COMENTAR NÃO É NECESSÁRIO!!!!!!!
    /* if (received_Ns != Ns) {  // Received the wanted frame (Ns requested)
      write_RR(fd);
    }
    else {*/
    
    write_REJ(fd, Ns_Recebido_Read);
    return -1;
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

  // Se o Ns recebido for igual ao Ns enviado pelo writer, então alterar o valor. Se não, descartar a mensagem. Enviar Ns de qualquer forma
  if(Ns_Recebido_Read == Ns_Enviado_Write) {
    /*
    if(erro == 400) {
      printf("ERRO1\n");
      Ns_Enviado_Write ^= 1;
    }
    */

    Ns_Enviado_Write ^= 1; // ou esperado
  }
  else {
    //s descartar mensagem
    j = 0;
  }
  printf("sent n %d\n", Ns_Enviado_Write);
  write_RR(fd, Ns_Enviado_Write);

  /*
  printf("Message size: %d\n", *sizeMessage);
  //message tem BCC2 no fim
  message = (unsigned char *)realloc(message, *sizeMessage - 1);

  *sizeMessage = *sizeMessage - 1;
  if (mandarDados)
  {
    if (trama == esperado)
    {
      esperado ^= 1;
    }
    else
      *sizeMessage = 0;
  }
  else
    *sizeMessage = 0;
  */

  return j - 6;
}

int llclose(struct applicationLayer *application) {
  if (application->status == TRANSMITTER) {
    write_DISC(*application);
    read_DISC(application->fileDescriptor);
    write_UA(*application);
    return 0;
  }
  else if (application->status == RECEIVER) {
    read_DISC(application->fileDescriptor);
    write_DISC(*application);
    read_UA(application->fileDescriptor);
    return 0;
  }
  return -1;
}
