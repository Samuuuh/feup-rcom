// ----- Interface Protocolo-Aplicação -----

struct applicationLayer;
struct linkLayer;

int llopen(struct applicationLayer *application);

int llwrite(struct applicationLayer *application, struct linkLayer *link);

int llread(struct applicationLayer *application, struct linkLayer *link);

int llclose(struct applicationLayer *application);

// -----------------------------------------