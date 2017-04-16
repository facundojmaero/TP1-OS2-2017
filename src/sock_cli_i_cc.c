#include "../include/funciones_cliente_cc.h"   

int initialize_udp_server_with_args(socklen_t *tamano_direccion , struct sockaddr_in* serv_addr){
	int sockudp, puerto;//, tamano_direccion;
	// unsigned char buffer[ TAM ];
	// struct sockaddr_in serv_addr;
	// int n;

	char *argv[] = {"" ,"6020"};

	sockudp = socket( AF_INET, SOCK_DGRAM, 0 );
	if (sockudp < 0) { 
		perror("ERROR en apertura de socket");
		exit( 1 );
	}

	memset( serv_addr, 0, sizeof(*serv_addr) );
	puerto = atoi( argv[1] );
	serv_addr->sin_family = AF_INET;
	serv_addr->sin_addr.s_addr = INADDR_ANY;
	serv_addr->sin_port = htons( puerto );
	memset( (serv_addr->sin_zero), '\0', 8 );

	if( bind( sockudp, (struct sockaddr *) serv_addr, sizeof(*serv_addr) ) < 0 ) {
		perror( "ERROR en binding" );
		exit( 1 );
	}

    printf( "Socket disponible: %d\n", ntohs(serv_addr->sin_port) );

	*tamano_direccion = sizeof( struct sockaddr );
	// int bytes_read;

	// // do {
	//      bytes_read = recvfrom( sockudp, (void*)buffer, TAM, 0, (struct sockaddr *)&serv_addr, &tamano_direccion );
	//  //     if(strcmp(buffer, "/END") == 0){
	// 	// 	// printf("recibi /END\n");
	// 	// 	break;
	// 	// }
	//      // if (bytes_read > 0) {
	//      	// int res = fprintf(fd, "%s",buffer);
	//         printf("Buffer: %.*s\n", bytes_read, buffer); 
 //    		bytes_read = sendto( sockudp, (void *)"/ACK", 5, 0, (struct sockaddr *)&serv_addr, tamano_direccion  );
	// 		if ( n < 0 ) {
	// 			perror( "escritura en socket" );
	// 			exit( 1 );
	// 		}
	// 		printf("envie /ACK\n");
	//      // }
	// // } while (bytes_read > 0);

	// // bytes_read = sendto( sockudp, (void *)"/ACK", 5, 0, (struct sockaddr *)&serv_addr, tamano_direccion  );
	// // if ( n < 0 ) {
	// // 	perror( "escritura en socket" );
	// // 	exit( 1 );
	// // }

	// // fclose(fd);
	// close(sockudp);
	return sockudp;
}   

void send_udp(int sockfd, char buffer[], struct sockaddr_in* serv_addr, socklen_t tamano_direccion){
	ssize_t n = sendto( sockfd, (void *)buffer, TAM, 0, (struct sockaddr *) serv_addr, tamano_direccion  );
	if ( n < 0 ) {
		perror( "send_udp" );
		exit( 1 );
	}
	return;
}

void recv_udp(int sockfd, char buffer[], struct sockaddr_in* serv_addr, socklen_t* tamano_direccion){
	memset( buffer, 0, TAM );
	ssize_t n = recvfrom( sockfd, buffer, TAM, 0, (struct sockaddr *) serv_addr, tamano_direccion);
	if ( n < 0 ) {
		perror( "recv_udp" );
		exit( 1 );
	}
	return;
}

void
recibir_datos(int sockfd){
	//levanto servidor UDP (se invierten los roles)

	socklen_t tamano_direccion;
	int sockudp;
	struct sockaddr_in serv_addr;
	sockudp = initialize_udp_server_with_args(&tamano_direccion , &serv_addr);

	//aviso al servidor que tengo la estructura udp lista
	sendToSocket(sockfd, "/UDP_READY");
	printf("envie udp ready\n");
	//

	char buffer[TAM];
	//espero filename
	recv_udp(sockudp, buffer, &serv_addr, &tamano_direccion);
	// readFromSocket(sockfd, buffer);
	printf("recibi filename %s\n",buffer );
	// sendToSocket(sockfd, "/ACK");
	send_udp(sockudp, "/ACK", &serv_addr, tamano_direccion);
	printf("envie ack filename\n");
	//

	//abro archivo con filename
	FILE *fd;
    fd = fopen(buffer, "wb");
	//

	//recibo lineas
	while(1){
		// printf("esperando en while\n");
		// readFromSocket(sockfd, buffer);
		recv_udp(sockudp, buffer, &serv_addr, &tamano_direccion);

		if(strcmp(buffer, "/FINISH") == 0){
			// sendToSocket(sockfd, "/ACK");
			send_udp(sockudp, "/ACK", &serv_addr, tamano_direccion);
			break;
			//termino la transmision
		}

		printf("while: %s\n", buffer);
	 	fprintf(fd, "%s\n",buffer);	
		//guardo en archivo
		// sendToSocket(sockfd,"/ACK");
		send_udp(sockudp, "/ACK", &serv_addr, tamano_direccion);
	}
	//

	//cierro el proceso
	printf("fin de funcion\n");
	close(sockudp);
	fclose(fd);
	return;
	//
}

int 
main( int argc, char *argv[] ) {
	int sockfd, puerto, socketResult;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int status;
	// char* line, *line2;
	char password[TAM], user_pw[TAM];
	char buffer[TAM];
	char* user = NULL, *ip = NULL, *port = NULL;

    char nada[2] = "";
    char arroba[2] = "@";
    char dots[2]=":";

    printf("\nPor favor ingrese usuario, ip y puerto\n"BOLDBLUE"$ "RESET);
	do{
		int result = 0;
		char *line = read_line();
		// char line[100] = "facundo@localhost:6020";
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
		char *line2 = read_line();
		// char line3[10];
		// strcpy(line3,"alfajor");
		strcpy(password,line2);
		strcpy(user_pw,user);
		strcat(user_pw," ");
		strcat(user_pw,password);
		// sscanf(line2,"%s",user_pw);
		// printf("%s\n", user_pw);

		//envio user + password a servidor
		sendToSocket(sockfd,user_pw);
		readFromSocket(sockfd,buffer);

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
		printf(BOLDBLUE "%s@%s $ "RESET, user,ip);
		fgets( buffer, TAM-1, stdin );

		if(strlen(buffer) == 1){
			continue;
		}

		sendToSocket(sockfd, buffer);

		while( 1 ){
			socketResult = readFromSocket(sockfd, buffer);
			if (strcmp(buffer, endMsg) == 0){
				break;
			}
			else if (strcmp(buffer, disconnectMsg) == 0){
				printf( "Finalizando ejecución\n" );
				exit(0);
			}
			else if(strcmp(buffer, "/START") == 0){
				//inicio rutina de recepcion de datos
				recibir_datos(sockfd);
				// initialize_udp_server();
				// descargar_estacion();

			}
			printf("%*.*s\n", socketResult, socketResult, buffer);
		}
		memset( buffer, '\0', TAM );
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
	if ((getline(&line, &bufsize, stdin)) != -1){
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
	return n;
}

