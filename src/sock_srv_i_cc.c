#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include "../include/funciones_servidor_cc.h"

int 
main( int argc, char *argv[] ) {
	int sockfd, newsockfd, pid;
	int status = 1;
	char buffer[TAM];
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;

	startServer(&sockfd, &clilen, &serv_addr, &cli_addr);

	while( 1 ) {
		newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
		if ( newsockfd < 0 ) {
			perror( "accept" );
			exit( 1 );
		}

		pid = fork(); 
		if ( pid < 0 ) {
			perror( "fork" );
			exit( 1 );
		}

		if ( pid == 0 ) {  // Proceso hijo
			close( sockfd );

			while ( status ) {
				memset( buffer, 0, TAM );
				readFromSocket(newsockfd,buffer);

				printf( "PROCESO %d. ", getpid() );
				printf( "Recibí: %s", buffer );

				char user[20];
				char password[20];

				sscanf(buffer,"%s%s",user,password);

				if(strcmp(user,correctUsername) || strcmp(password,correctPassword)){
					//no son iguales
					sendToSocket(newsockfd, errormsg);
					printf("error password\n");
					close(newsockfd);
					exit(0);
				}
				else{
					sendToSocket(newsockfd,okmsg);
					printf("good password\n");
					status=0;
				}
			}
			sleep(1);

			sendToSocket(newsockfd, "Opciones disponibles:\n"
									"	listar\n"
									"	descargar <nro_estacion>\n"
									"	diario_precipitacion <nro_estacion>\n"
									"	mensual_precipitacion <nro_estacion>\n"
									"	promedio <variable>\n"
									"	desconectar\n"
									"	ayuda\n");

			while ( 1 ) {
				readFromSocket(newsockfd,buffer);

				printf( "PROCESO %d. ", getpid() );
				printf( "Recibí: %s", buffer );

				sendToSocket(newsockfd, "Obtuve su mensaje");

				// Verificación de si hay que terminar
				buffer[strlen(buffer)-1] = '\0';
				if( !strcmp( "fin", buffer ) ) {
					printf( "PROCESO %d. Como recibí 'fin', termino la ejecución.\n\n", getpid() );
					exit(0);
				}
			}
		}
		else {
			printf( "SERVIDOR: Nuevo cliente, que atiende el proceso hijo: %d\n", pid );
			close( newsockfd );
		}
	}
	return 0; 
} 

void 
sendToSocket(int sockfd, char cadena[]){
	int n = write( sockfd, cadena, strlen(cadena));
	if ( n < 0 ) {
		perror( "escritura de socket" );
		exit( 1 );
	}
}

void 
readFromSocket(int sockfd, char buffer[]){
	memset( buffer, '\0', TAM );
	int n = read( sockfd, buffer, TAM-1 );
	if ( n < 0 ) {
		perror( "lectura de socket" );
		exit( 1 );
	}
}

void 
startServer(int* sockfd, socklen_t* clilen, struct sockaddr_in* serv_addr,struct sockaddr_in* cli_addr){
	int puerto;

	*sockfd = socket( AF_INET, SOCK_STREAM, 0);
	if ( *sockfd < 0 ) { 
		perror( " apertura de socket ");
		exit( 1 );
	}

	memset( (char *) serv_addr, 0, sizeof(*serv_addr) );
	puerto = 6020;
	serv_addr->sin_family = AF_INET;
	serv_addr->sin_addr.s_addr = INADDR_ANY;
	serv_addr->sin_port = htons( puerto );

	if ( bind(*sockfd, ( struct sockaddr *) serv_addr, sizeof( *serv_addr ) ) < 0 ) {
		perror( "ligadura" );
		exit( 1 );
	}

        printf( "Proceso: %d - socket disponible: %d\n", getpid(), ntohs(serv_addr->sin_port) );

	listen( *sockfd, 5 );
	*clilen = sizeof( *cli_addr );
}