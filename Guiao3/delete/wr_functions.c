#include "llfunctions.h"
#include "const_defines.h"

int llopen(int fd, int status) {
    if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 1; /* inter-unsigned character timer unused */
    newtio.c_cc[VMIN] = 0;  /* blocking read until 5 unsigned chars received */

    /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
    */

    tcflush(fd, TCIOFLUSH);


    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");


    unsigned char c;
    do {
        sendControlMessage(fd, SET_C);
        alarm(TIMEOUT);
        flagAlarm = 0;
        int state = 0;

        while (!paragem && !flagAlarm)
        {
        read(fd, &c, 1);
        stateMachineUA(&state, &c);
        }
    } while (flagAlarm && sumAlarms < NUMMAX);
    
    printf("flag alarm %d\n", flagAlarm);
    printf("soma %d\n", sumAlarms);
    
    if (flagAlarm && sumAlarms == 3) {
        return FALSE;
    }
    else {
        flagAlarm = FALSE;
        sumAlarms = 0;
        return TRUE;
    }
}

int llwrite(int fd, char * buffer, int length) {

}

int llread(int fd, char * buffer) {

}

int llclose(int fd) {
    
}