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
// C_RR - Defines framing of type RR (receiver ready / positive ACK) - R0000101
// C_REJ - Defines framing of type REJ (reject / negative ACK) - R0000001

/* BCC - Protection Camp - A^C (XOR / Exclusive OR)
 *
 * BCC_SET - BCC for framing of type SET (set up) - A^C_SET
 * BCC_DISC - BCC for framing of type DISC (disconnect) - A^C_DISC
 * BCC_UA - BCC for framing of type UA (unnumbered acknowledgment) - A^C_UA
 * BCC_RR - BCC for framing of type RR (receiver ready / positive ACK) - A^C_RR
 * BCC_REJ - BCC for framing of type REJ (reject / negative ACK) - A^C_REJ
*/

// --------------- Defines --------------------

// Boolean values
#define FALSE 0
#define TRUE 1

// Used in struct termios
#define BAUDRATE B38400

// Used in messages SET and UA
#define BCC_SET A_Sender_Receiver^C_SET
#define BCC_UA A_Sender_Receiver^C_UA

// Used in struct applicationLayer
#define RECEIVER 0
#define TRANSMITTER 1

// Used in struct linkLayer
#define MAX_SIZE 256

// ---------- Structures Declaration ----------

int fd_write;

// Estados
enum current_state {start, flag_rcv, a_rcv, c_rcv, bcc_ok, stop};

// Aplicação
struct applicationLayer {
  int fileDescriptor; //Descritor correspondente à porta série*/
  int status;   // TRANSMITTER | RECEIVER
};

// Protocolo
struct linkLayer {
  char port[20];  //Dispositivo /dev/ttySx, x = 0, 1
  int baudRate;   //Velocidade de transmissão
  unsigned int sequenceNumber;   //Número de sequência da trama: 0, 1
  unsigned int timeout; //Valor do temporizador: 1 s
  unsigned int numTransmissions; //Número de tentativas em caso defalha
  char frame[MAX_SIZE]; //Trama
};

// --------------------------------------------
