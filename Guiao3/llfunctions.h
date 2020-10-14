// ----- Interface Protocolo-Aplicação -----

int llopen(int fd, int x);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char * buffer);

int llclose(int fd);

// -----------------------------------------