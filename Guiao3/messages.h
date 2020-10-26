// ----- Check BCC Functions -----

unsigned char calculateBCC2(unsigned char *message, int sizeMessage);

// ----- Input/Output Messages -----

void write_SET(int fd);

void read_SET(int fd);

void write_UA(int fd);

void read_UA(int fd);

void write_DISC(int fd);

void read_DISC(int fd);

void write_RR(int fd);

int read_RR(int fd);  // Returns received Ns

void write_REJ(int fd);

// ---------------------------------