// ----- Interface Protocolo-Aplicação -----

int llopen(struct applicationLayer *application);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char * buffer);

int llclose(int fd);

// -----------------------------------------