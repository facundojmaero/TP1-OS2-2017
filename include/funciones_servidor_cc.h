#define TAM 256
const char correctUsername[20] = "facundo";
const char correctPassword[20] = "alfajor";
#define errormsg "ERROR"
#define okmsg "OK"

void sendToSocket(int sockfd, char cadena[]);
void readFromSocket(int sockfd, char buffer[]);
void startServer(int* sockfd, socklen_t* clilen, struct sockaddr_in* serv_addr,struct sockaddr_in* cli_addr);
