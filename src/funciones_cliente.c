#include "../include/comunes.h"
#include "../include/funciones_cliente_cc.h"

int 
initialize_udp_server_with_args(socklen_t *tamano_direccion , struct sockaddr_in* serv_addr){
	int sockudp, puerto;
	char *argv[] = {"6020"};

	sockudp = socket( AF_INET, SOCK_DGRAM, 0 );
	if (sockudp < 0) { 
		perror("ERROR en apertura de socket");
		exit( 1 );
	}

	memset( serv_addr, 0, sizeof(*serv_addr) );
	puerto = atoi( argv[0] );
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
	return sockudp;
}   

void
recibir_datos(int sockfd){
	//levanto servidor UDP (se invierten los roles)

	socklen_t tamano_direccion;
	int sockudp;
	struct sockaddr_in serv_addr;
	sockudp = initialize_udp_server_with_args(&tamano_direccion , &serv_addr);

	//aviso al servidor que tengo la estructura udp lista
	send_to_socket(sockfd, udp_ready);
	printf("envie udp ready\n");
	//

	char buffer[TAM];
	//espero filename
	recv_udp(sockudp, buffer, &serv_addr, &tamano_direccion);
	// read_from_socket(sockfd, buffer);
	printf("recibi filename %s\n",buffer );
	// send_to_socket(sockfd, ack_msg);
	send_udp(sockudp, ack_msg, &serv_addr, tamano_direccion);
	printf("envie ack filename\n");
	//

	//abro archivo con filename
	FILE *fd;
    fd = fopen(buffer, "wb");
	//

	//recibo lineas
	while(1){
		// printf("esperando en while\n");
		// read_from_socket(sockfd, buffer);
		recv_udp(sockudp, buffer, &serv_addr, &tamano_direccion);

		if(strcmp(buffer, end_UDP_Msg) == 0){
			// send_to_socket(sockfd, ack_msg);
			send_udp(sockudp, ack_msg, &serv_addr, tamano_direccion);
			break;
			//termino la transmision
		}

		printf("%s\n", buffer);
	 	fprintf(fd, "%s\n",buffer);	
		//guardo en archivo
		// send_to_socket(sockfd,ack_msg);
		send_udp(sockudp, ack_msg, &serv_addr, tamano_direccion);
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