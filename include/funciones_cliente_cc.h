#define TAM 500//256
#define endMsg "/END"
#define disconnectMsg "/BYE"

int parser(char* line, char before[], char after[], char** buff);
char *read_line(void);
int sendToSocket(int sockfd, char cadena[]);
int readFromSocket(int sockfd, char buffer[]);