// ----- State Machines Functions -----

enum current_state;

int process_SET(char received, enum current_state *state);

int process_UA(char received, enum current_state *state);

int process_DISC(char received, enum current_state *state);

int process_DATA(char* message, int index, enum current_state *state);

int process_RR_REJ(unsigned char received, enum current_state *state);

// ------------------------------------