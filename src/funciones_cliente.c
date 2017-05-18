/** @file funciones_cliente.c
 *  @brief Archivo principal de funciones del cliente.
 *
 *  Contiene las funciones que utiliza el cliente para poder comunicarse
 *  e interactuar correctamente con el servidor.
 *
 *  @author Facundo Maero
 */



#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "../include/comunes.h"
#include "../include/funciones_cliente_cc.h"

/**
* @brief Inicializa un servidor UDP para la transferencia de logs (puerto 6020).
*
* @param *tamano_direccion El tamaño de la direccion del servidor (4 para IPv4).
* @param *dest_addr Estructura de tipo sockaddr_in donde guardar informacion
* sobre servidor.
* @return sockudp El descriptor del socket para poder escribir en el y enviar
* datos al servidor.
*/
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
get_ip_address(char address[], int fd){
	struct ifreq ifr;

 /* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

 /* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, "enp4s0", IFNAMSIZ-1);

	ioctl(fd, SIOCGIFADDR, &ifr);

 /* display result */
	// printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	strcpy(address, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

/**
* @brief Recibe datos de descarga de archivos desde el servidor.
*
* Recibe datos del servidor, una vez solicitada la descarga de una estación.
* Primero inicializa un servidor UDP. Avisa al servidor cuando esté listo, y
* recibe el nombre del archivo a crear.
* Luego, lo crea y comienza a recibir datos y escribirlos allí. Luego de cada
* lectura del socket se envía un mensaje de acreditación, para asegurar que los
* mensajes lleguen en orden y de manera correcta.
* La transferencia dura hasta que se reciba un flag de fin de transmisión, momento
* en el que se cierra el socket y el archivo, y finaliza la subrutina.
*
* @param sockfd File Descriptor del socket UDP a utilizar.
*/
void
recibir_datos(int sockfd){
	//levanto servidor UDP (se invierten los roles)

	socklen_t tamano_direccion;
	int sockudp;
	struct sockaddr_in serv_addr;
	sockudp = initialize_udp_server_with_args(&tamano_direccion , &serv_addr);

	//aviso al servidor que tengo la estructura udp lista
	send_to_socket(sockfd, udp_ready);
	
	//////////////////////////////////////////////
	//Agregado, negociacion de la direccion IP
	//en tiempo de ejecucion.
	//Antes de recibir el nombre del archivo por UDP
	//envio un ultimo mensaje por TCP con la IP del cliente
	char address[100];
	get_ip_address(address, sockfd);

	send_to_socket(sockfd, address);
	//////////////////////////////////////////////

	char buffer[TAM];
	//espero filename
	recv_udp(sockudp, buffer, &serv_addr, &tamano_direccion);
	send_udp(sockudp, ack_msg, &serv_addr, tamano_direccion);
	//

	//abro archivo con filename
	FILE *fd;
    fd = fopen(buffer, "wb");
	//

    printf("Descargando archivo '%s'\n",buffer);
	//recibo lineas
	while(1){
		recv_udp(sockudp, buffer, &serv_addr, &tamano_direccion);

		if(strcmp(buffer, end_UDP_Msg) == 0){
			send_udp(sockudp, ack_msg, &serv_addr, tamano_direccion);
			break;
			//termino la transmision
		}
	 	fprintf(fd, "%s\n",buffer);	
		//guardo en archivo
		send_udp(sockudp, ack_msg, &serv_addr, tamano_direccion);
	}
	//

	//cierro el proceso
	printf("Transferencia exitosa!\n");
	close(sockudp);
	fclose(fd);
	return;
	//
}

/**
* @brief Parsea input del usuario en busca de user, ip y puerto.
*
* Busca un token entre los delimitadores before y after en la cadena line.
*
* @param *line Cadena ingresada por el usuario.
* @param before[] delimitador antes del token a buscar.
* @param after[] delimitador después del token a buscar.
* @param **buff string donde se devuelve el token encontrado
* @return 0 si se encontró un token, -1 caso contrario.
*/
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

/**
* @brief Lee input desde teclado (por stdin), hasta un \n.
*
* @return Linea leida.
*/
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