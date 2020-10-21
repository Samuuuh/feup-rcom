// ----- Check BCC Functions -----

unsigned char calculateBCC2(unsigned char *message, int sizeMessage);

// ----- Input/Output Messages -----

void write_SET(int fd);

void read_SET(int fd);

void write_UA(int fd);

void read_UA(int fd);

void write_DISC(int fd);

void read_DISC(int fd);

// ---------------------------------