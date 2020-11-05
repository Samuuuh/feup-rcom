
// ----- Check BCC2 Functions -----

unsigned char calculateBCC2All(unsigned char *message, int sizeMessage);

unsigned char calculateBCC2(unsigned char *message, int sizeMessage);

// ----- Input/Output Messages -----

struct applicationLayer;

void write_SET(int fd);

void read_SET(int fd);

void write_UA(struct applicationLayer app);

int read_UA(int fd);  // TRUE if UA was received, FALSE otherwise

void write_DISC(struct applicationLayer app);

void read_DISC(int fd);

void write_RR(int fd, int Ns);

void write_REJ(int fd, int Ns);

int read_RR(int fd, int* received_NS);  // TRUE if RR was received, and sets received_NS value



// ---------------------------------