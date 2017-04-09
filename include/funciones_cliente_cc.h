#define TAM 256

int parser(char* line, char before[], char after[], char** buff);
char *read_line(void);
void sendToSocket(int sockfd, char cadena[]);
void readFromSocket(int sockfd, char buffer[]);