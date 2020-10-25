/*Non-Canonical Input Processing*/

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "const_defines.h"
#include "llfunctions.h"
#include "math.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

volatile int STOP=FALSE;

extern struct termios oldtio;

extern int fd_write;

unsigned char UA[5];

int main(int argc, char** argv)
{ 
  if ( (argc < 2) || 
        ((strcmp("/dev/ttyS0", argv[1])!=0) && 
        (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n"); 
    exit(1);
  }

  struct applicationLayer application;

  application.status = TRANSMITTER;
  strncpy(application.port, argv[1], sizeof(application.port));

  if (llopen(&application) < 0) {
    printf("LLOPEN() failed");
    exit(2);
  }
  
  printf("LLOPEN() done successfully\n\n");

  unsigned char data[] = { 0x55, 0x84, 0x66, 0x44, 0x55, 0x55, 0x77, 0x55, 0x77, 0x66 };

  // Calculate the number of Data Packets to send
  int packet_number = strlen(data) / MAX_SIZE;
  packet_number += ((strlen(data) % MAX_SIZE) ? 1 : 0);

  // Create Start Control Packet
  unsigned char start_packet[128];
  sprintf(start_packet, "%d%d%d", 2, 0, 1);
  long int digits_V = (long int) log10(sizeof(data)) + 1;
  unsigned char V_string[64];
  sprintf(V_string, "%ld", sizeof(data));
  for (int i = 0 ; i < digits_V; i++) {
    start_packet[i + 3] = V_string[i];
  }

  // Send Start Control Packet
  if (llwrite(application.fileDescriptor, start_packet, 3 + digits_V) < 0) {
    printf("LLWRITE() failed");
    exit(3);
  }

  // Send Data Packets
  unsigned char data_packet[128];
  long int bytes_to_process = sizeof(data);
  int data_ind = 0;
  for (int i = 1; i <= packet_number; i++) {
    memset(data_packet, 0, sizeof (data_packet));
    int K = MIN(MAX_SIZE, bytes_to_process);
    sprintf(data_packet, "%d%d%d%d", 1, i % 255, K / 256, K % 256);   // C, N, L2, L1
    bytes_to_process -= K;
    for (int j = 0; j < K; j++) {
      data_packet[j + 4] = data[data_ind];
      data_ind++;
    }
    if (llwrite(application.fileDescriptor, data_packet, 4 + K) < 0) {
      printf("LLWRITE() failed");
      exit(3);
    }
  }
/*
   for(int i = 1; i <= packet_number; i++) {
    struct applicationPacket packet;
    packet.controlCamp = 1;
    packet.sequenceNumber = i;
    int K = MIN(MAX_SIZE, total_size);
    printf("K = %d\n", K);
    packet.L2 = (K / 256);
    packet.L1 = (K % 256);
    
    for(int j = 0; j < K; j++) {
      packet.data[j] = data[data_index];
      data_index++;
      total_size--;
    }

    printf("total_size = %d\n", total_size);

    packets[i] = packet;
  }
*/

  // Create End Control Packet
  unsigned char end_packet[128];
  sprintf(end_packet, "%d%d%d", 3, 0, 1);
  digits_V = (long int) log10(sizeof(data)) + 1;
  sprintf(V_string, "%ld", sizeof(data));
  for (int i = 0 ; i < digits_V; i++) {
    end_packet[i + 3] = V_string[i];
  }

  // Send End Control Packet
  if (llwrite(application.fileDescriptor, end_packet, 3 + digits_V) < 0) {
    printf("LLWRITE() failed");
    exit(3);
  }

  // ------------------

  // File Size Parameter
  /*struct parameter param;
  param.T = 0;
  param.L = 1;
  printf("STRLEN = %ld\n", strlen(data));
  unsigned char len[64];
  sprintf(len, "%ld", strlen(data));
  strncpy(param.V, len, sizeof(param.V));*/

  //printf("O V = %s\n", param.V);

  // Add Start Control Packet
  /*struct applicationPacket controlPacket;
  controlPacket.controlCamp = 2;
  controlPacket.parameters[0] = param;
  controlPacket.number_parameters = 1;
  packets[0] = controlPacket;*/
  
  /*int data_index = 0;
  int total_size = 10;   // strlen(data) DA MAL???
  printf("SIZE OF INICIO = %ld\n", sizeof(data));

  printf("PACKET NUMBER = %d\n", packet_number);
  for(int i = 1; i <= packet_number; i++) {
    struct applicationPacket packet;
    packet.controlCamp = 1;
    packet.sequenceNumber = i;
    int K = MIN(MAX_SIZE, total_size);
    printf("K = %d\n", K);
    packet.L2 = (K / 256);
    packet.L1 = (K % 256);
    
    for(int j = 0; j < K; j++) {
      packet.data[j] = data[data_index];
      data_index++;
      total_size--;
    }

    printf("total_size = %d\n", total_size);

    packets[i] = packet;
  }*/

  // Add End Control Packet
  /*controlPacket.controlCamp = 3;
  packets[packet_number + 1] = controlPacket;

  printf("Start Packet (C = %d): Parameters: ", packets[0].controlCamp);
  printf("Start (T = %d), ", packets[0].parameters[0].T);
  printf("Start (L = %d), ", packets[0].parameters[0].L);
  printf("V = %s\n", packets[0].parameters[0].V);

  for (unsigned int i = 1 ; i <= packet_number; i++) {
    printf("Data Packet %d (C = %d): L1 = 0x%02x, L2 = 0x%02x with data = ",packets[i].sequenceNumber, packets[i].controlCamp, packets[i].L1, packets[i].L2);
    for (unsigned int j = 0; j < (256 * packets[i].L2 + packets[i].L1); j++) {
      printf("0x%02x ", packets[i].data[j]);
    }
    printf("\n");
  }

  printf("End Packet (C = %d): Parameters: ", packets[packet_number + 1].controlCamp);
  printf("End (T = %d), ", packets[packet_number + 1].parameters[0].T);
  printf("End (L = %d), ", packets[packet_number + 1].parameters[0].L);
  printf("V = %s\n", packets[packet_number + 1].parameters[0].V);*/

  // ---------------------- .-.
  /*fflush(stdin);
  unsigned char frame_data[256] = "";
  
  printf("control_camp = %d\n", packets[0].controlCamp);
  sprintf(frame_data, "%d", packets[0].controlCamp);
  printf("frame_data = %s\n", frame_data);
  for (int i = 0; i < packets[0].number_parameters; i++) {
    unsigned char param_string[256];
    sprintf(param_string, "%d%d%s", packets[0].parameters[i].T, packets[0].parameters[i].L, packets[0].parameters[i].V);
     printf("T: %d\n", packets[0].parameters[i].T);
     printf("L: %d\n", packets[0].parameters[i].L);
     printf("V: %s\n", packets[0].parameters[i].V);
    printf("PARAM STRING: %s\n", param_string);
    strcat(frame_data, param_string);
    printf("FRAME DATA: %s\n", frame_data);
  }

  for (int l = 0 ; l < 5 ; l++) {
    printf("frame_data[%d] = %0x%02x\n", l, frame_data[l]);
  }*/

  /*strncpy(link.frame, frame_data, sizeof(link.frame));
  link.baudRate = sizeof(link.frame);*/

  /*if (llwrite(&application, &link) < 0) {
    printf("LLWRITE() failed");
    exit(3);
  }*/

  /*for (unsigned int i = 0 ; i < packet_number + 2; i++) {
       
    memset(frame_data, 0, sizeof (frame_data));

    if (llwrite(&application, &link) < 0) {
      printf("LLWRITE() failed");
      exit(3);
    }
  }*/

  // Send End Packet
  /*memset(frame_data, 0, sizeof (frame_data));
  sprintf(frame_data, "%c", packets[packet_number + 1].controlCamp);
  for (int i = 0; i < packets[packet_number + 1].number_parameters; i++) {
    unsigned char param_string[256];
    sprintf(param_string, "%c%c%s", packets[packet_number + 1].parameters[i].T, packets[packet_number + 1].parameters[i].L, packets[packet_number + 1].parameters[i].V);
    strcat(frame_data, param_string);
  }*/

  //strncpy(link.frame, frame_data, sizeof(link.frame));

  /*if (llwrite(&application, &link) < 0) {
    printf("LLWRITE() failed");
    exit(3);
  }*/

  if (llclose(&application) < 0) {
    printf("LLCLOSE() failed");
    exit(4);
  }

  printf("LLCLOSE() done successfully\n");

  sleep(1);
  
  if (tcsetattr(application.fileDescriptor,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(application.fileDescriptor);
  return 0;
}