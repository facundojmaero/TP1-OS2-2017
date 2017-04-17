/** @file funciones_io.c
 *  @brief Funciones de envio y recepción por sockets.
 *
 *	Conjunto de funciones usadas por el cliente y el servidor para comunicarse
 * 	entre si. Hay una versión TCP y una UDP de enviar y recibir.
 *
 *  @author Facundo Maero
 */

#include "../include/comunes.h"

/**
* @brief Envia un mensaje utilizando conexion no segura (UDP)
*
* Dado un mensaje contenido en un buffer, lo envía por un socket utilizando
* la función de transmisión no segura (sin conexión).
*
* @param sockfd Descriptor a donde enviar el mensaje.
* @param buffer[] El mensaje a enviar.
* @param serv_addr
* @param tamano_direccion
*/
void 
send_udp(int sockfd, char buffer[], struct sockaddr_in* serv_addr, socklen_t tamano_direccion){
    ssize_t n = sendto( sockfd, (void *)buffer, TAM, MSG_CONFIRM, (struct sockaddr *) serv_addr, tamano_direccion  );
    if ( n < 0 ) {
        perror( "send_udp" );
        exit( 1 );
    }
    return;
}

/**
* @brief Recibe un mensaje utilizando conexion no segura (UDP)
*
* Lee un socket utilizando la función de transmisión no segura (sin conexión).
*
* @param sockfd Descriptor de donde leer el mensaje.
* @param buffer[] String donde guardar el mensaje.
* @param serv_addr
* @param tamano_direccion
*/
void 
recv_udp(int sockfd, char buffer[], struct sockaddr_in* serv_addr, socklen_t* tamano_direccion){
    memset( buffer, 0, TAM );
    ssize_t n = recvfrom( sockfd, buffer, TAM, MSG_WAITALL, (struct sockaddr *) serv_addr, tamano_direccion);
    if ( n < 0 ) {
        perror( "recv_udp" );
        exit( 1 );
    }
    return;
}

/**
* @brief Envia un mensaje utilizando conexion segura (TCP)
*
* Dado un mensaje contenido en un buffer, lo envía por un socket utilizando
* la función de transmisión segura (con conexión).
* Primero envía el tamaño del mensaje, y luego el mensaje propiamente dicho.
*
* @param sockfd Descriptor a donde enviar el mensaje.
* @param buffer[] El mensaje a enviar.
* @return La cantidad de bytes enviados.
*/
int 
send_to_socket(int sockfd, char buffer[]){
	int datalen = strlen(buffer);
	int tmp = htonl(datalen);
	int n = write(sockfd, (char*)&tmp, sizeof(tmp));
	if (n < 0) perror("ERROR writing to socket");
	n = write(sockfd, buffer, datalen);
	if (n < 0) perror("ERROR writing to socket");
	return n;
}

/**
* @brief Recibe un mensaje utilizando conexion segura (TCP)
*
* Lee un socket utilizando la función de transmisión segura (con conexión).
* Primero recibe el tamaño del mensaje, y luego el mensaje propiamente dicho.
*
* @param sockfd Descriptor desde donde leer el mensaje.
* @param buffer[] Arreglo de chars donde guardar el mensaje recibido.
* @return La cantidad de bytes recibidos.
*/
int 
read_from_socket(int sockfd,char buffer[]){
	memset( buffer, '\0', TAM );
	int buflen;
	int n = read(sockfd, (char*)&buflen, sizeof(buflen));
	if (n < 0) perror("ERROR reading from socket");
	buflen = ntohl(buflen);
	n = read(sockfd, buffer, buflen);
	if (n < 0) perror("ERROR reading from socket");
	return n;
}