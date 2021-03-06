/** @file funciones_server.c
 *  @brief Archivo con funciones extras del servidor.
 *
 *  Contiene funciones extras que necesita el servidor, pero que no están
 *  relacionadas directamente con la comunicación con el cliente.
 *
 *  @author Facundo Maero
 */

#include "../include/comunes.h"
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

/**
* @brief Inicializa el servidor TCP.
*
* Crea un socket TCP con los argumentos que recibe. Utiliza el puerto 6020.
* @args *sockfd
* @args clilen
* @args *serv_addr
* @args *cli_addr
*/
void 
start_server(int* sockfd, socklen_t* clilen, struct sockaddr_in* serv_addr,struct sockaddr_in* cli_addr){
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

/**
* @brief Tokeniza una cadena de caracteres segun delimitadores.
*
* Toma un String y busca tokens según una lista de delimitadores (LSH_TOK_DELIM).
*
* Crea un socket TCP con los argumentos que recibe. Utiliza el puerto 6020.
* @args *line String a tokenizar.
* @args **tokens Puntero a punteros que representan a cada token (modificado por referencia).
* @return El numero de tokens encontrado.
*/
int
split_line(char *line, char** tokens)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
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
  return position;
}

/**
* @brief Saltea lineas en un archivo.
*
* Dado un descriptor de archivo y un numero de lineas, las saltea leyéndolas
* y descartando el resultado.
*
* Crea un socket TCP con los argumentos que recibe. Utiliza el puerto 6020.
* @args *stream Descriptor del archivo donde se quieren saltar lineas.
* @args lines Cantidad de lineas a saltar.
*/
int 
skip_lines(FILE* stream, int lines){
    
    for (int i = 0; i < lines; ++i)
    {
        fscanf(stream, "%*[^\n]\n");
    }
    return 0;
}