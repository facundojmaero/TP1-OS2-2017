#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <stddef.h>
#include "../include/colors.h"

#define TAM 512
#define errormsg "ERROR"
#define okmsg "OK"
#define endMsg "/END"
#define disconnectMsg "/BYE"
#define argErrorMsg "Comando incorrecto"
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

const char correctUsername[20] = "facundo";
const char correctPassword[20] = "alfajor";
const int inicioEstaciones = 2;
const int NRO_ESTACIONES = 5;
const int NRO_SENSORES = 16;
const char * functionNames[] = {
    "ayuda",
    "desconectar",
    "listar",
    "mensual_precip",
    "diario_precip",
    "descargar",
    "promedio"
};
const int NRO_FUNCIONES = (sizeof (functionNames) / sizeof (const char *));

char welcome_message[TAM] = "Opciones disponibles:\n"
									"	* listar\n"
									"	* descargar <nro_estacion>\n"
									"	* diario_precip <nro_estacion>\n"
									"	* mensual_precip <nro_estacion>\n"
									"	* promedio <variable>\n"
									"	* desconectar\n"
									"	* ayuda\n";

struct SensorDisponible
{
    int esta;
    char nombreSensor[30];
};

struct DatoEstacion 
{
   char    fecha[50];
   char    dia[20];
   float   temp;
   float   humedad;
   float   ptoRocio;
   float   precip;
   float   velocViento;
   char    direcViento[30];
   float   rafagaMax;
   float   presion;
   float   radiacion;
   float   tempSuelo1;
   float   tempSuelo2;
   float   tempSuelo3;
   float   humedadSuelo1;
   float   humedadSuelo2;
   float   humedadSuelo3;
   float   humedadHoja;
}; 

struct Estacion 
{
    struct DatoEstacion dato[4100];
    int index;
    int cantElem;
    struct SensorDisponible sensores[16];
    int     numero;
    char    nombre[50];
    int     idLocalidad;
};

int sendToSocket(int sockfd, char cadena[]);
int readFromSocket(int sockfd, char buffer[]);
void startServer(int* sockfd, socklen_t* clilen, struct sockaddr_in* serv_addr,struct sockaddr_in* cli_addr);
int split_line(char *line, char** tokens);
void mensualPrecipitacion(struct Estacion estaciones[], int nroEstacion, int newsockfd);
void argError(int newsockfd);