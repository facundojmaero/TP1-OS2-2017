#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include "../include/colors.h"
#define TAM 512
#define endMsg "/END"
#define disconnectMsg "/BYE"

int parser(char* line, char before[], char after[], char** buff);
char *read_line(void);
int sendToSocket(int sockfd, char cadena[]);
int readFromSocket(int sockfd, char buffer[]);