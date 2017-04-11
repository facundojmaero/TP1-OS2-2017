#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include "../include/funciones_servidor_cc.h"

void 
mensualPrecipitacion(struct Estacion estaciones[], int nroEstacion, int newsockfd){
	int socketResult;
    float precipAcumulada=0;
    int i;
    for (i = 0; i < estaciones[nroEstacion].cantElem; ++i)
    {
        precipAcumulada+=estaciones[nroEstacion].dato[i].precip;
    }
    printf("Precipitacion acumulada mensual.\nEstacion %s: %.1f mm\n",
        estaciones[nroEstacion].dato[0].estacion,precipAcumulada);
    char mensaje[TAM];
    snprintf(mensaje, TAM, "Precipitacion acumulada mensual.\nEstacion %s: %.1f mm\n",
    	estaciones[nroEstacion].dato[0].estacion,precipAcumulada);
    socketResult = sendToSocket(newsockfd, mensaje);
    socketResult = sendToSocket(newsockfd, endMsg);
}

void 
listarEstaciones(struct Estacion estaciones[], int newsockfd){
    char mensaje[TAM];
    int socketResult;
    printf("Estaciones disponibles:\n\n");
    snprintf(mensaje, TAM, "Estaciones disponibles:\n");
    socketResult = sendToSocket(newsockfd, mensaje);
    for (int i = 0; i < NRO_ESTACIONES; ++i)
    {
        printf("%d) %s. Sensores con datos:\n",i,estaciones[i].dato[0].estacion);
        snprintf(mensaje,TAM,"%d) %s. Sensores con datos:",i,estaciones[i].dato[0].estacion);
        socketResult = sendToSocket(newsockfd, mensaje);
        for (int j = 0; j < NRO_SENSORES; ++j)
        {
            if(estaciones[i].sensores[j].esta){
                printf("    %s\n", estaciones[i].sensores[j].nombreSensor);
                snprintf(mensaje,TAM,"    %s", estaciones[i].sensores[j].nombreSensor);
                socketResult = sendToSocket(newsockfd, mensaje);
            }
        }
        printf("\n\n");
        snprintf(mensaje, TAM, "\n");
        socketResult = sendToSocket(newsockfd, mensaje);
    }
    socketResult = sendToSocket(newsockfd, endMsg);
}

int 
skipLines(FILE* stream, int lines){
    
    for (int i = 0; i < lines; ++i)
    {
        fscanf(stream, "%*[^\n]\n");
    }
    return 0;
}

void 
verificarSensores(struct Estacion stationArray[], int j, char* line2, 
    struct SensorDisponible nombreTemporal[] ){
    char* tempstr = calloc(strlen(line2)+1, sizeof(char));
    strcpy(tempstr, line2);
    const char s[2] = ",";
    char *token;
    int cuenta=0, skip=0;

    for (int i = 0; i < 16; ++i)
    {
        strcpy(stationArray[j].sensores[i].nombreSensor,nombreTemporal[i].nombreSensor);
    }

    token = strtok(tempstr, s);
    while( token != NULL )
    {
        if(skip<4){
            skip++;
            token = strtok(NULL, s);
            continue;
        }

        if(!strcmp(token,"\n")){
            break;
        }

        if(!strcmp(token, "--")){
            stationArray[j].sensores[cuenta].esta = 0;
        }
        else{
            stationArray[j].sensores[cuenta].esta = 1;   
        }
        // printf("%d Sensor %s tiene un %d\n",cuenta,stationArray[j].sensores[cuenta].nombreSensor, stationArray[j].sensores[cuenta].esta);
        token = strtok(NULL, s);
        cuenta++;
    }
}

int 
main( int argc, char *argv[] ) {
	int sockfd, newsockfd, pid, status = 1, socketResult;
	char buffer[TAM];
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;

	startServer(&sockfd, &clilen, &serv_addr, &cli_addr);

	// FILE* stream = fopen("../datos_meteorologicos.CSV", "r");
    FILE* stream = fopen("../prueba1.CSV", "r");
    if (stream == NULL) {
      perror("Error opening CSV table");
      exit(EXIT_FAILURE);
    }

    size_t characters = 0;
    size_t len = 0;
    char* line2 = NULL;
    char* nombreColumnas = NULL;
    int skipResult;

    const char s[2] = ",";
    char *token;
    struct Estacion stationArray[6];
    int i=0;//fila del archivo
    int j=0;//numero de estacion
    // char *fechaDia;
    // char espacio[2] = " ";
    int result,idEstacion;

    skipResult = skipLines(stream,inicioEstaciones-1);
    skipResult++;//borrar
    characters = getline(&nombreColumnas,&len,stream);
    struct SensorDisponible nombreTemporal[16];

    int cuenta=0;
    token = strtok(nombreColumnas, s);
    while( token != NULL ) {
        if(cuenta>3){
        strcpy(nombreTemporal[cuenta-4].nombreSensor,token);
        // printf( "%s\n", nombreTemporal[cuenta].nombreSensor);
        }
        token = strtok(NULL, s);
        cuenta++;
    }

    while((characters = getline(&line2,&len,stream)) != -1 ){     
        
        result = sscanf(line2,"%d",&idEstacion);
        if (i != 0){
            //Si no estoy llenando la primer fila
            //Comparo token (id de estacion) con id de estacion anterior para
            //ver si pase a otra estacion
            if(idEstacion != stationArray[j].dato[i-1].numero){
                //Si el ID es distinto al anterior, 
                //termine de guardar una estacion y paso a la siguiente
                //Guardo el tamaño de la estacion (cant lineas)
                stationArray[j].cantElem = i;
                stationArray[j].index = j;      

                j++;
                i=0;
                if(j==5){break;}
            }
        }

        result = sscanf(line2, "%d,%[^','],%f,%[^','],%f,%f,%f"
                                   ",%f,%f,%[^','],%f,%f,%f,%f,%f"
                                   ",%f,%f,%f,%f,%f",
                                    &stationArray[j].dato[i].numero,
                                    stationArray[j].dato[i].estacion,
                                    &stationArray[j].dato[i].id,
                                    stationArray[j].dato[i].fecha,
                                    &stationArray[j].dato[i].temp,
                                    &stationArray[j].dato[i].humedad, 
                                    &stationArray[j].dato[i].ptoRocio,
                                    &stationArray[j].dato[i].precip,
                                    &stationArray[j].dato[i].velocViento,
                                    stationArray[j].dato[i].direcViento,
                                    &stationArray[j].dato[i].rafagaMax,
                                    &stationArray[j].dato[i].presion,
                                    &stationArray[j].dato[i].radiacion,
                                    &stationArray[j].dato[i].tempSuelo1,
                                    &stationArray[j].dato[i].tempSuelo2,
                                    &stationArray[j].dato[i].tempSuelo3,
                                    &stationArray[j].dato[i].humedadSuelo1,
                                    &stationArray[j].dato[i].humedadSuelo2,
                                    &stationArray[j].dato[i].humedadSuelo3,
                                    &stationArray[j].dato[i].humedadHoja);
        sscanf(stationArray[j].dato[i].fecha,"%s",stationArray[j].dato[i].dia);
        result++;//borrar
        if(i==0){
            verificarSensores(stationArray, j, line2,nombreTemporal);
        }
        i++;

    //Guardo la cantidad de elementos de la ultima estacion
    stationArray[j].cantElem = i;
    stationArray[j].index = j;
	}

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
				socketResult = readFromSocket(newsockfd,buffer);

				printf( "PROCESO %d. ", getpid() );
				printf("Recibí: %*.*s\n", socketResult, socketResult, buffer);

				char user[20];
				char password[20];

				sscanf(buffer,"%s%s",user,password);

				if(strcmp(user,correctUsername) || strcmp(password,correctPassword)){
					//no son iguales
					socketResult = sendToSocket(newsockfd, errormsg);
					printf("\nerror password\n");
					close(newsockfd);
					exit(0);
				}
				else{
					socketResult = sendToSocket(newsockfd,okmsg);
					printf("\ngood password\n");
					status=0;
				}
			}

			socketResult = sendToSocket(newsockfd, "Opciones disponibles:\n"
									"	listar\n"
									"	descargar <nro_estacion>\n"
									"	diario_precipitacion <nro_estacion>\n"
									"	mensual_precipitacion <nro_estacion>\n"
									"	promedio <variable>\n"
									"	desconectar\n"
									"	ayuda\n");

			while ( 1 ) {
				socketResult = readFromSocket(newsockfd, buffer);

				printf( "PROCESO %d. ", getpid() );
				printf("Recibí: %*.*s\n", socketResult, socketResult, buffer);

				char **args = split_line(buffer);
				////////////////////////////////////////////////////////////////
				i=0;
			    while(args[i]!=NULL){
			        i++;
			    }

			    if(!strcmp(args[0],"listar") && args[1] == NULL){
			        listarEstaciones(stationArray, newsockfd);
			    }
			    // if(!strcmp(args[0],"desconectar") && args[1] == NULL){
			        //desconectar()
			    // }
			    // if(!strcmp(args[0],"descargar") && strlen(args[1]) == 1){
			    //     char caracter;
			    //     int numero;
			    //     strcpy(&caracter,args[1]);
			    //     if(isdigit(caracter)){
			    //         numero = atoi(&caracter);
			    //         descargarEstacion(numero,stream,stationArray[numero]);
			    //     }
			    // }
			    // if(!strcmp(args[0],"diario_precip") && strlen(args[1]) == 1){
			    //     char caracter;
			    //     int numero;
			    //     strcpy(&caracter,args[1]);
			    //     if(isdigit(caracter)){
			    //         numero = atoi(&caracter);
			    //         diarioPrecipitacion(stationArray[numero],numero);
			    //     }
			    // }
			    if(!strcmp(args[0],"mensual_precip") && strlen(args[1]) == 1){
			        char caracter;
			        int numero;
			        strcpy(&caracter,args[1]);
			        if(isdigit(caracter)){
			            numero = atoi(&caracter);
			            mensualPrecipitacion(stationArray,numero, newsockfd);
			        }
			    }
			    // if(!strcmp(args[0],"promedio") && strlen(args[1])){
			    //     char caracter;
			    //     int numero;
			    //     strcpy(&caracter,args[1]);
			    //     if(isdigit(caracter)){
			    //         numero = atoi(&caracter);
			    //         // mensualPrecipitacion(stationArray[numero],numero);
			    //     }
			    // }
				////////////////////////////////////////////////////////////////
				else{
					socketResult = sendToSocket(newsockfd, buffer);
					socketResult = sendToSocket(newsockfd, endMsg);
				}
				// Verificación de si hay que terminar
				buffer[strlen(buffer)-1] = '\0';
				if( !strcmp( "desconectar", buffer ) ) {
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

char **split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
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