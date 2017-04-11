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
#include "../include/funciones_cliente_cc.h"

int sendToSocket2(int sockfd, char cadena[]);
int readFromSocket2(int sockfd, char cadena[], int tam);

int 
main( int argc, char *argv[] ) {
	int sockfd, puerto, socketResult;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int terminar = 0, status;
	// char* line, *line2;
	char password[TAM], user_pw[TAM];
	char buffer[TAM];
	char* user = NULL, *ip = NULL, *port = NULL;
    int result=0;

    char nada[2] = "";
    char arroba[2] = "@";
    char dots[2]=":";

    printf("\nPor favor ingrese usuario, ip y puerto\n"BOLDBLUE"$ "RESET);
	do{
		result = 0;
		// line = read_line();
		char line[100] = "facundo@localhost:6020";
		result += parser(line,dots,nada,&port);
    	result += parser(line,arroba,dots,&ip);
    	result += parser(line,nada,arroba,&user);

	    if(result<0){
	        fprintf(stderr, "Comando incorrecto\n"BOLDBLUE"$ "RESET);
	        status = 1;
	        continue;
	    }

		puerto = atoi( port );
		sockfd = socket( AF_INET, SOCK_STREAM, 0 );
		if ( sockfd < 0 ) {
			perror( "ERROR apertura de socket" );
			status = 1;
			continue;
			// exit( 1 );
		}

		server = gethostbyname( ip );
		if (server == NULL) {
			fprintf( stderr,"Error, no existe el host\n"BOLDBLUE"$ "RESET);
			status = 1;
			continue;
			// exit( 0 );
		}
		memset( (char *) &serv_addr, '0', sizeof(serv_addr) );
		serv_addr.sin_family = AF_INET;
		bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
		// serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons( puerto );
		if ( connect( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr ) ) < 0 ) {
			perror( "conexion" );
			status = 1;
			continue;
			// exit( 1 );
		}//si llegue aca estoy conectado al servidor

		printf("Inserte password: \n"BOLDBLUE"$ "RESET);
		// line2 = read_line();
		char line3[10];
		strcpy(line3,"alfajor");
		strcpy(password,line3);
		strcpy(user_pw,user);
		strcat(user_pw," ");
		strcat(user_pw,password);

		printf("%s\n", user_pw);

		//envio user + password a servidor
		socketResult = sendToSocket(sockfd,user_pw);
		socketResult = readFromSocket(sockfd,buffer);

		if(!strcmp(buffer,"ERROR")){
			printf("Nombre de usuario y/o contraseña incorrecto\n"BOLDBLUE"$ "RESET);
			status = 1;
			continue;
		}

		if(!strcmp(buffer,"OK")){
			printf("Bienvenido %s\n", user);
			status = 0;
		}
	}while(status);

	socketResult = readFromSocket(sockfd,buffer);
	printf("%*.*s\n", socketResult, socketResult, buffer);

	while(1) {

		printf(BOLDBLUE "%s@%s$ "RESET, user,ip);
		fgets( buffer, TAM-1, stdin );

		socketResult = sendToSocket(sockfd, buffer);

		// Verificando si se escribió: fin
		buffer[strlen(buffer)-1] = '\0';
		if( !strcmp( "desconectar", buffer ) ) {
			terminar = 1;
		}

		while( 1 ){
			socketResult = readFromSocket(sockfd, buffer);
			if(!strcmp(buffer, endMsg)){
				break;
			}
			printf("%*.*s\n", socketResult, socketResult, buffer);
		}


		if( terminar ) {
			printf( "Finalizando ejecución\n" );
			exit(0);
		}
	}
	return 0;
} 


int 
parser(char* line, char before[], char after[], char** buff){
    *buff = NULL;
    if(strstr(line,before)!=NULL){

        *buff = strstr(line,before) + strlen(before);
        if((*buff = strtok(*buff,after))!=NULL){
            return 0;
        }
        else
            return -1;
    }
    else
        return -1;
}

char *
read_line(void){
	char *line = NULL;
	size_t bufsize = 0;
	ssize_t read;
	if ((read = getline(&line, &bufsize, stdin)) != -1){
		return line;
	}
	else{
		perror("Error reading line");
		exit(0);
	}
}

int 
sendToSocket(int sockfd, char buffer[]){
	int datalen = strlen(buffer);
	int tmp = htonl(datalen);
	int n = write(sockfd, (char*)&tmp, sizeof(tmp));
	if (n < 0) perror("ERROR writing to socket");
	n = write(sockfd, buffer, datalen);
	if (n < 0) perror("ERROR writing to socket");
	return n;
}

int 
readFromSocket(int sockfd,char buffer[]){
	memset( buffer, '\0', TAM );
	int buflen;
	int n = read(sockfd, (char*)&buflen, sizeof(buflen));
	if (n < 0) perror("ERROR reading from socket");
	buflen = ntohl(buflen);
	n = read(sockfd, buffer, buflen);
	if (n < 0) perror("ERROR reading from socket");
	// else printf("%*.*s\n", n, n, buffer);
	return n;
}

