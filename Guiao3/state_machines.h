// ----- State Machines Functions -----

enum current_state;

int process_SET(char received, enum current_state *state);

int process_UA(char received, enum current_state *state);

// ------------------------------------