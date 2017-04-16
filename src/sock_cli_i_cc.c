/** @file sock_cli_i_cc.c
 *  @brief Archivo principal del Cliente.
 *
 *  Contiene el programa principal que ejecutará el cliente.
 *
 *  @author Facundo Maero
 */

#include "../include/comunes.h"
#include "../include/funciones_cliente_cc.h"

/**
* @brief Función principal del Cliente.
*
* Crea un socket de tipo TCP con la IP y el puerto deseados, se conecta con
* el servidor, proporciona usuario y contraseña. Se encarga de enviar las 
* instrucciones al mismo y mostrar los resultados por consola.
*/
int 
main( int argc, char *argv[] ) {
	int sockfd, puerto, socketResult, status;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char password[TAM], user_pw[TAM], buffer[TAM];
	char* user = NULL, *ip = NULL, *port = NULL;

    char nada[2] = "";
    char arroba[2] = "@";
    char dots[2]=":";

	/*!< Bucle de binding y autenticación con el servidor.*/
    printf("\nPor favor ingrese usuario, ip y puerto\n"BOLDBLUE"$ "RESET);
	do
	{
		/*!<  */
		int result = 0;

		/*!< Se espera input al estilo usuario@IP:puerto */
		char *line = read_line();
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
		}

		server = gethostbyname( ip );
		if (server == NULL) {
			fprintf( stderr,"Error, no existe el host\n"BOLDBLUE"$ "RESET);
			status = 1;
			continue;
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
		}

		/*!< Una vez conectado con el servidor, se solicita la contraseña. */
		printf("Inserte password: \n"BOLDBLUE"$ "RESET);
		char *line2 = read_line();
		strcpy(password,line2);
		strcpy(user_pw,user);
		strcat(user_pw," ");
		strcat(user_pw,password);

		/*!< envio user + password a servidor */
		send_to_socket(sockfd,user_pw);
		/*!< Leo la respuesta */
		read_from_socket(sockfd,buffer);

		/*!< Si fue un error, notifica al usuario y pide los datos nuevamente. */
		if(!strcmp(buffer,"ERROR")){
			printf("Nombre de usuario y/o contraseña incorrecto\n"BOLDBLUE"$ "RESET);
			status = 1;
			continue;
		}

		/*!< Caso contrario da la bienvenida y pasa a la sección siguiente. */
		if(!strcmp(buffer,"OK")){
			printf("Bienvenido %s\n", user);
			status = 0;
		}
	}
	while(status);

	/*!< Lee el mensaje de bienvenida del server */
	socketResult = read_from_socket(sockfd,buffer);
	printf("%*.*s\n", socketResult, socketResult, buffer);

	/*!< Bucle principal de la aplicación */
	while(1) {
		/*!< Imprime el prompt y lee el comando del usuario */
		printf(BOLDBLUE "%s@%s $ "RESET, user,ip);
		fgets( buffer, TAM-1, stdin );

		if(strlen(buffer) == 1){
			continue;
		}

		/*!< Lo envia al servidor y empieza a leer respuestas */
		send_to_socket(sockfd, buffer);

		while( 1 ){
			socketResult = read_from_socket(sockfd, buffer);
			/*!< Lee indefinidamente hasta que llegue un flag de fin de mensaje */
			if (strcmp(buffer, endMsg) == 0){
				break;
			}
			/*!< Si solicite una desconexión, el servidor devuelve el ok
			 y el proceso ciente finaliza correctamente. */
			else if (strcmp(buffer, disconnectMsg) == 0){
				printf( "Finalizando ejecución\n" );
				exit(0);
			}
			/*!< Si se solicitó una descarga por UDP, se recibe el flag correspondiente
			y se llama a la función que gestiona la transferencia. */
			else if(strcmp(buffer, start_UDP_Msg) == 0){
				recibir_datos(sockfd);

			}
			printf("%*.*s\n", socketResult, socketResult, buffer);
		}
		memset( buffer, '\0', TAM );
	}
	return 0;
} 