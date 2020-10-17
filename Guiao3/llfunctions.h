// ----- Interface Protocolo-Aplicação -----

struct applicationLayer;

int llopen(struct applicationLayer *application);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char * buffer);

int llclose(struct applicationLayer *application);

// -----------------------------------------