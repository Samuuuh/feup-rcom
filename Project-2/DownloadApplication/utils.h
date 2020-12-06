struct hostent;

enum code_state {start, first_digit, second_digit, third_digit, last_line, code_received};

enum pasv_state {pasv_start, h1, h2, h3, h4, p1, p2, pasv_end};

int parseArguments(char* argument, char* user, char* pass, char* host, char* file_path);

struct hostent* getIP(char *host);

void readServerResponse(int sockfd, char *response, char *fullResponse);

int login(int sockfd, char *user, char *pass);

int activatePassiveMode(int sockfd);

int download_file(int sockfd, int sockfd_client, char* file_path);