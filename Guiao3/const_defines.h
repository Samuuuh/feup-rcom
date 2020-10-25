// ---------- Format of Framing (tramas) ----------

/*
 *  Types of framing:
 *   - Information framing:
 *     | F | A | C | BCC1 | D1 | Data | Dn | BCC2 | F |
 *
 *   - Supervision framing and Unnumbered framing:
 *     | F | A | C | BCC1 | F |
 *
*/

#define FLAG 0x7E   // Delimitation of framing (tramas)

#define A_Sender_Receiver 0x03    // Commands sent by Sender and answers sent by Receiver
#define A_Receiver_Sender 0x01    // Commands sent by Receiver and answers sent by Sender

#define C_SET 0x03    // Defines framing of type SET (set up)
#define C_DISC 0x0B    // Defines framing of type DISC (disconnect)
#define C_UA 0x07    // Defines framing of type UA (unnumbered acknowledgment)
#define C_RR(N) (N << 7 | 0b101)  // C_RR - Defines framing of type RR (N = 0 -> 0x05, N = 1 -> 0x85)
#define C_REJ(N) (N << 7 | 0b1)  //C_REJ - Defines framing of type REJ (N = 0 -> 0x01, N = 1 -> 0x81)

/* BCC - Protection Camp - A^C (XOR / Exclusive OR)
 *
 * BCC_SET - BCC for framing of type SET (set up) - A^C_SET
 * BCC_DISC - BCC for framing of type DISC (disconnect) - A^C_DISC
 * BCC_UA - BCC for framing of type UA (unnumbered acknowledgment) - A^C_UA
 * BCC_RR - BCC for framing of type RR (receiver ready / positive ACK) - A^C_RR
 * BCC_REJ - BCC for framing of type REJ (reject / negative ACK) - A^C_REJ
*/

#define BCC_SET (A_Sender_Receiver ^ C_SET)
#define BCC_UA (A_Sender_Receiver ^ C_UA)
#define BCC_DISC (A_Sender_Receiver ^ C_DISC)
#define BCC_RR(N) (A_Sender_Receiver ^ C_RR(N))
#define BCC_REJ(N) (A_Sender_Receiver ^ C_REJ(N))

// --------------- Defines --------------------

// Boolean values
#define FALSE 0
#define TRUE 1

// Used in struct termios
#define BAUDRATE B38400

// Used in struct applicationLayer
#define RECEIVER 0
#define TRANSMITTER 1

// Used in struct linkLayer
#define MAX_SIZE 8

// ESC byte, used in byte stuffing
#define ESC 0x7D

// MIN: Find minimal element, used in llwrite()
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// ---------- Structures Declaration ----------

int fd_write;

// Estados
enum current_state {start, flag_rcv, a_rcv, c_rcv, bcc_ok, data_rcv, bcc2_ok, stop, finished};

// Aplicação
struct applicationLayer {
  char port[20];  //Dispositivo /dev/ttySx, x = 0, 1
  int fileDescriptor; //Descritor correspondente à porta série*/
  int status;   // TRANSMITTER | RECEIVER
};

// Parameter Struct of Control Packets
struct parameter {
  unsigned char T, L;
  unsigned char V[128];
};

// Pacote de Aplicação
struct applicationPacket {
  int controlCamp;

  // Control Packet
  struct parameter parameters[4];
  int number_parameters;

  // Data Packet
  int sequenceNumber;
  unsigned char L1, L2;
  unsigned char data[MAX_SIZE];
};

// Protocolo
struct linkLayer {
  char port[20];  //Dispositivo /dev/ttySx, x = 0, 1
  int baudRate;   //Velocidade de transmissão
  unsigned int sequenceNumber;   //Número de sequência da trama: 0, 1
  unsigned int timeout; //Valor do temporizador: 1 s
  unsigned int numTransmissions; //Número de tentativas em caso defalha
  unsigned char frame[128]; //Trama
};

// --------------------------------------------
