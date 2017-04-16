#include "../include/comunes.h"

void 
send_udp(int sockfd, char buffer[], struct sockaddr_in* serv_addr, socklen_t tamano_direccion){
    ssize_t n = sendto( sockfd, (void *)buffer, TAM, 0, (struct sockaddr *) serv_addr, tamano_direccion  );
    if ( n < 0 ) {
        perror( "send_udp" );
        exit( 1 );
    }
    return;
}

void 
recv_udp(int sockfd, char buffer[], struct sockaddr_in* serv_addr, socklen_t* tamano_direccion){
    memset( buffer, 0, TAM );
    ssize_t n = recvfrom( sockfd, buffer, TAM, 0, (struct sockaddr *) serv_addr, tamano_direccion);
    if ( n < 0 ) {
        perror( "recv_udp" );
        exit( 1 );
    }
    return;
}

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