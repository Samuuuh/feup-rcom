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

int llwrite(int fd, char * buffer, int length) {

}

/*int LLWRITE(int fd, unsigned char *mensagem, int size)
{
  unsigned char BCC2;
  unsigned char *BCC2Stuffed = (unsigned char *)malloc(sizeof(unsigned char));
  unsigned char *mensagemFinal = (unsigned char *)malloc((size + 6) * sizeof(unsigned char));
  int sizeMensagemFinal = size + 6;
  int sizeBCC2 = 1;
  BCC2 = calculoBCC2(mensagem, size);
  BCC2Stuffed = stuffingBCC2(BCC2, &sizeBCC2);
  int rejeitado = FALSE;

  mensagemFinal[0] = FLAG;
  mensagemFinal[1] = A;
  if (trama == 0)
  {
    mensagemFinal[2] = C10;
  }
  else
  {
    mensagemFinal[2] = C11;
  }
  mensagemFinal[3] = (mensagemFinal[1] ^ mensagemFinal[2]);

  int i = 0;
  int j = 4;
  for (; i < size; i++)
  {
    if (mensagem[i] == FLAG)
    {
      mensagemFinal = (unsigned char *)realloc(mensagemFinal, ++sizeMensagemFinal);
      mensagemFinal[j] = Escape;
      mensagemFinal[j + 1] = escapeFlag;
      j = j + 2;
    }
    else
    {
      if (mensagem[i] == Escape)
      {
        mensagemFinal = (unsigned char *)realloc(mensagemFinal, ++sizeMensagemFinal);
        mensagemFinal[j] = Escape;
        mensagemFinal[j + 1] = escapeEscape;
        j = j + 2;
      }
      else
      {
        mensagemFinal[j] = mensagem[i];
        j++;
      }
    }
  }

  if (sizeBCC2 == 1)
    mensagemFinal[j] = BCC2;
  else
  {
    mensagemFinal = (unsigned char *)realloc(mensagemFinal, ++sizeMensagemFinal);
    mensagemFinal[j] = BCC2Stuffed[0];
    mensagemFinal[j + 1] = BCC2Stuffed[1];
    j++;
  }
  mensagemFinal[j + 1] = FLAG;

  //mandar mensagem
  do
  {

    unsigned char *copia;
    copia = messUpBCC1(mensagemFinal, sizeMensagemFinal); //altera bcc1
    copia = messUpBCC2(copia, sizeMensagemFinal);         //altera bcc2
    write(fd, copia, sizeMensagemFinal);

    flagAlarm = FALSE;
    alarm(TIMEOUT);
    unsigned char C = readControlMessageC(fd);
    if ((C == CRR1 && trama == 0) || (C == CRR0 && trama == 1))
    {
      printf("Recebeu rr %x, trama = %d\n", C, trama);
      rejeitado = FALSE;
      sumAlarms = 0;
      trama ^= 1;
      alarm(0);
    }
    else
    {
      if (C == CREJ1 || C == CREJ0)
      {
        rejeitado = TRUE;
        printf("Recebeu rej %x, trama=%d\n", C, trama);
        alarm(0);
      }
    }
  } while ((flagAlarm && sumAlarms < NUMMAX) || rejeitado);
  if (sumAlarms >= NUMMAX)
    return FALSE;
  else
    return TRUE;
}*/

int llread(int fd, char * buffer) {

}

/*unsigned char *LLREAD(int fd, int *sizeMessage)
{
  unsigned char *message = (unsigned char *)malloc(0);
  *sizeMessage = 0;
  unsigned char c_read;
  int trama = 0;
  int mandarDados = FALSE;
  unsigned char c;
  int state = 0;

  while (state != 6)
  {
    read(fd, &c, 1);
    //printf("%x\n",c);
    switch (state)
    {
    //recebe flag
    case 0:
      if (c == FLAG)
        state = 1;
      break;
    //recebe A
    case 1:
      //printf("1state\n");
      if (c == A)
        state = 2;
      else
      {
        if (c == FLAG)
          state = 1;
        else
          state = 0;
      }
      break;
    //recebe C
    case 2:
      //printf("2state\n");
      if (c == C10)
      {
        trama = 0;
        c_read = c;
        state = 3;
      }
      else if (c == C11)
      {
        trama = 1;
        c_read = c;
        state = 3;
      }
      else
      {
        if (c == FLAG)
          state = 1;
        else
          state = 0;
      }
      break;
    //recebe BCC
    case 3:
      //printf("3state\n");
      if (c == (A ^ c_read))
        state = 4;
      else
        state = 0;
      break;
    //recebe FLAG final
    case 4:
      //printf("4state\n");
      if (c == FLAG)
      {
        if (checkBCC2(message, *sizeMessage))
        {
          if (trama == 0)
            sendControlMessage(fd, RR_C1);
          else
            sendControlMessage(fd, RR_C0);

          state = 6;
          mandarDados = TRUE;
          printf("Enviou RR, T: %d\n", trama);
        }
        else
        {
          if (trama == 0)
            sendControlMessage(fd, REJ_C1);
          else
            sendControlMessage(fd, REJ_C0);
          state = 6;
          mandarDados = FALSE;
          printf("Enviou REJ, T: %d\n", trama);
        }
      }
      else if (c == Escape)
      {
        state = 5;
      }
      else
      {
        message = (unsigned char *)realloc(message, ++(*sizeMessage));
        message[*sizeMessage - 1] = c;
      }
      break;
    case 5:
      //printf("5state\n");
      if (c == escapeFlag)
      {
        message = (unsigned char *)realloc(message, ++(*sizeMessage));
        message[*sizeMessage - 1] = FLAG;
      }
      else
      {
        if (c == escapeEscape)
        {
          message = (unsigned char *)realloc(message, ++(*sizeMessage));
          message[*sizeMessage - 1] = Escape;
        }
        else
        {
          perror("Non valid character after escape character");
          exit(-1);
        }
      }
      state = 4;
      break;
    }
  }
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
  return message;
}*/

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

/*void LLCLOSE(int fd) -- read
{
  readControlMessage(fd, DISC_C);
  printf("Recebeu DISC\n");
  sendControlMessage(fd, DISC_C);
  printf("Mandou DISC\n");
  readControlMessage(fd, UA_C);
  printf("Recebeu UA\n");
  printf("Receiver terminated\n");

  tcsetattr(fd, TCSANOW, &oldtio);
}*/

/*void LLCLOSE(int fd) -- write
{
  sendControlMessage(fd, DISC);
  printf("Mandou DISC\n");
  unsigned char C;
  //espera ler o DISC
  C = readControlMessageC(fd);
  while (C != DISC)
  {
    C = readControlMessageC(fd);
  }
  printf("Leu DISC\n");
  sendControlMessage(fd, UA_C);
  printf("Mandou UA final\n");
  printf("Writer terminated \n");

  if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }
}*/