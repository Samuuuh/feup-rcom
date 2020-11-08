// ----- Interface Protocolo-Aplicação -----

struct applicationLayer;

int llopen(struct applicationLayer *application);

int llwrite(int fd, unsigned char* buffer, int length);

int llread(int fd, unsigned char* buffer);

int llclose(struct applicationLayer *application);

// -----------------------------------------