#include "../include/funciones_servidor_cc.h"

size_t
getVariableOffset(char variable[], int* indiceSensor){
    if(strcmp(variable, "temp") == 0){
        *indiceSensor = 0;
        return (offsetof(struct DatoEstacion, temp));
    }
    else if(strcmp(variable, "humedad") == 0){
        *indiceSensor = 1;
        return (offsetof(struct DatoEstacion, humedad));
    }
    else if(strcmp(variable, "ptoRocio") == 0){
        *indiceSensor = 2;
        return (offsetof(struct DatoEstacion, ptoRocio));
    }
    else if(strcmp(variable, "precip") == 0){
        *indiceSensor = 3;
        return (offsetof(struct DatoEstacion, precip));
    }
    else if(strcmp(variable, "velocViento") == 0){
        *indiceSensor = 4;
        return (offsetof(struct DatoEstacion, velocViento));
    }
    else if(strcmp(variable, "rafagaMax") == 0){
        *indiceSensor = 6;
        return (offsetof(struct DatoEstacion, rafagaMax));
    }
    else if(strcmp(variable, "presion") == 0){
        *indiceSensor = 7;
        return (offsetof(struct DatoEstacion, presion));
    }
    else if(strcmp(variable, "radiacion") == 0){
        *indiceSensor = 8;
        return (offsetof(struct DatoEstacion, radiacion));
    }
    else if(strcmp(variable, "tempSuelo1") == 0){
        *indiceSensor = 9;
        return (offsetof(struct DatoEstacion, tempSuelo1));
    }
    else if(strcmp(variable, "tempSuelo2") == 0){
        *indiceSensor = 10;
        return (offsetof(struct DatoEstacion, tempSuelo2));
    }
    else if(strcmp(variable, "tempSuelo3") == 0){
        *indiceSensor = 11;
        return (offsetof(struct DatoEstacion, tempSuelo3));
    }
    else if(strcmp(variable, "humedadSuelo1") == 0){
        *indiceSensor = 12;
        return (offsetof(struct DatoEstacion, humedadSuelo1));
    }
    else if(strcmp(variable, "humedadSuelo2") == 0){
        *indiceSensor = 13;
        return (offsetof(struct DatoEstacion, humedadSuelo2));
    }
    else if(strcmp(variable, "humedadSuelo3") == 0){
        *indiceSensor = 14;
        return (offsetof(struct DatoEstacion, humedadSuelo3));
    }
    else if(strcmp(variable, "humedadHoja") == 0){
        *indiceSensor = 15;
        return (offsetof(struct DatoEstacion, humedadHoja));
    }
    else{
    	*indiceSensor = -1;
        return -1;
    }
}

int
promediar(struct Estacion stationArray[], char variable[], int newsockfd){
    size_t longitudElemento = offsetof(struct Estacion, dato[1]);
    longitudElemento = longitudElemento / 4;
    //lo convierto a unidades de 4 bytes (float)

    int indiceSensor;
    size_t offset;
    char mensaje[TAM];
    offset = getVariableOffset(variable, &indiceSensor);
    if(offset > longitudElemento*4){
        printf("variable inexistente\n");
        return -1;
    }

    printf("Promedios variable "BOLDGREEN"%s:"RESET"\n", variable);
    snprintf(mensaje, TAM, "Promedios variable "BOLDGREEN"%s:"RESET"\n", variable);
    sendToSocket(newsockfd, mensaje);

    for(int j=0; j < NRO_ESTACIONES; j++){
        if(stationArray[j].sensores[indiceSensor].esta == 0){
            printf("%d - %s: No se encuentran datos.\n",stationArray[j].numero, stationArray[j].nombre);
            snprintf(mensaje,TAM, "%d - %s: "BOLDRED"No se encuentran datos."RESET,stationArray[j].numero, stationArray[j].nombre);
            sendToSocket(newsockfd, mensaje);
            continue;
        }
        float* ptr = (float*)((char*)&stationArray[j] + offset);
        //apunto al primer elemento de la primer estacion, la medicion
        //del primer dia de la variable que quiero promediar

        float suma = 0;

        for (int i = 0; i < stationArray[j].cantElem; ++i)
        {
            suma += *(float*)(ptr+(i*longitudElemento));
        }
        
        suma = suma/stationArray[j].cantElem;
        printf("%d - %s: %.1f\n", stationArray[j].numero, stationArray[j].nombre,suma);
        snprintf(mensaje, TAM, "%d - %s: %.1f", stationArray[j].numero, stationArray[j].nombre,suma);;
        sendToSocket(newsockfd, mensaje);
    }
    sendToSocket(newsockfd, endMsg);
    return 0;	
}

int
check_estacion_existente(struct Estacion estaciones[], int *nro){
    for (int i = 0; i < NRO_ESTACIONES; ++i)
    {
        if(estaciones[i].numero == *nro){
            *nro = i;
            return 1;
        }		
    }	
    return 0;
}

void 
diarioPrecipitacion(struct Estacion estaciones[], int nro, int newsockfd){
	if(check_estacion_existente(estaciones, &nro) == 0){
		sendToSocket(newsockfd, "Numero de estacion inexistente");
    	sendToSocket(newsockfd, endMsg);
		return;
	}

    float precipAcumulada=0;
    char mensaje[TAM];
    int i;
    printf("Precipitacion acumulada por dia.\n%d - Estacion %s:\n",
    	estaciones[nro].numero, estaciones[nro].nombre);
    snprintf(mensaje, TAM, "Precipitacion acumulada por dia.\n%d - Estacion %s:",estaciones[nro].numero, estaciones[nro].nombre);
    sendToSocket(newsockfd, mensaje);
    for (i = 0; i < estaciones[nro].cantElem; ++i)
    {
        if(i!=0 && (strcmp(estaciones[nro].dato[i].dia, estaciones[nro].dato[i-1].dia))){
            //termine de recorrer un dia. Sumo todo, muestro y reseteo contador
            printf("	%s: %.1f mm\n", estaciones[nro].dato[i-1].dia, precipAcumulada);
            snprintf(mensaje, TAM, "	%s: %.1f mm", estaciones[nro].dato[i-1].dia, precipAcumulada);
    		sendToSocket(newsockfd, mensaje);
            precipAcumulada = 0;
        }
        precipAcumulada+=estaciones[nro].dato[i].precip;
    }
    printf("	%s: %.1f mm\n", estaciones[nro].dato[i-1].dia, precipAcumulada);
    snprintf(mensaje, TAM, "	%s: %.1f mm", estaciones[nro].dato[i-1].dia, precipAcumulada);
    sendToSocket(newsockfd, mensaje);
    sendToSocket(newsockfd, endMsg);
}

void 
mensualPrecipitacion(struct Estacion estaciones[], int nro, int newsockfd){	
	if(check_estacion_existente(estaciones, &nro) == 0){
		sendToSocket(newsockfd, "Numero de estacion inexistente");
    	sendToSocket(newsockfd, endMsg);
		return;
	}
	float precipAcumulada=0;

    for (int i = 0; i < estaciones[nro].cantElem; ++i)
    {
        precipAcumulada+=estaciones[nro].dato[i].precip;
    }
    printf("Precipitacion acumulada mensual.\n%d - Estacion %s: %.1f mm\n", estaciones[nro].numero,estaciones[nro].nombre,precipAcumulada);
    char mensaje[TAM];

    snprintf(mensaje, TAM, "Precipitacion acumulada mensual.\n%d - Estacion %s: %.1f mm\n", estaciones[nro].numero,estaciones[nro].nombre,precipAcumulada);
	sendToSocket(newsockfd, mensaje);
    sendToSocket(newsockfd, endMsg);
}

void 
listarEstaciones(struct Estacion estaciones[], int newsockfd){
    char mensaje[TAM];
    printf("Estaciones disponibles:\n\n");
    snprintf(mensaje, TAM, "Estaciones disponibles:\n");
    sendToSocket(newsockfd, mensaje);
    for (int i = 0; i < NRO_ESTACIONES; ++i)
    {
        printf("%d - %s. Sensores con datos:\n",estaciones[i].numero,estaciones[i].nombre);
        snprintf(mensaje,TAM,"%d -  %s. Sensores con datos:",estaciones[i].numero,estaciones[i].nombre);
        sendToSocket(newsockfd, mensaje);
        for (int j = 0; j < NRO_SENSORES; ++j)
        {
            if(estaciones[i].sensores[j].esta){
                printf("    %s\n", estaciones[i].sensores[j].nombreSensor);
                snprintf(mensaje,TAM,"    %s", estaciones[i].sensores[j].nombreSensor);
                sendToSocket(newsockfd, mensaje);
            }
        }
        printf("\n\n");
        snprintf(mensaje, TAM, "\n");
        sendToSocket(newsockfd, mensaje);
    }
    sendToSocket(newsockfd, endMsg);
}

void showHelp(int newsockfd){
	char mensaje[2*TAM];
	strcpy(mensaje,"Ayuda:\n * listar:\n"
	"	Muestra un listado de todas las estaciones que hay en la base de datos,\n" 
	"	y muestra de que sensores tienen datos en cada una.\n"
	"* descargar "BOLDGREEN"<nro_estacion>:\n"RESET"" 
	"	Descarga un archivo con todos los datos de <nro_estacion>, llamado\n"
	"	\"estacion.CSV\".\n"
	"* mensual_precip "BOLDGREEN"<nro_estacion>:\n"RESET""
	"	Muestra el acumulado mensual de la variable precipitacion de la estacion\n"
	"	dada.\n"
	"* diario_precip "BOLDGREEN"<nro_estacion>:\n"RESET""
	"	Muestra el acumulado diario de la variable precipitacion de la estacion\n"
	"	dada.\n"
	"* promedio "BOLDGREEN"<variable>:\n"RESET""
	"	Muestra el promedio de las muestras de la variable de cada estacion.\n"
	"* desconectar:\n" 
	"	Termina la sesion del usuario.\n"RESET);

	sendToSocket(newsockfd, mensaje);
	sendToSocket(newsockfd, endMsg);
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
        token = strtok(NULL, s);
        cuenta++;
    }
    free(tempstr);
}

void
descargarEstacion(int numero, struct Estacion stationArray[], int newsockfd){
    if(numero >= NRO_ESTACIONES){
    perror("Intentando guardar estacion inexistente");
    exit(EXIT_FAILURE);
    }

    char buffer[TAM];

    //envio aviso inicio de transferencia a cliente
    sendToSocket(newsockfd, "/START");
    sendToSocket(newsockfd, endMsg);

    //aca levanto el cliente UDP (se invierten los roles)

    //

    //envio filename y espero ack
    sendToSocket(newsockfd,"descarga.txt");
	readFromSocket(newsockfd, buffer); //leo el ack   
    //

    //envio lineas mientras haya datos para enviar, y espero ack cada vez
	char linea [350];
    for(int i=0;i<stationArray[numero].cantElem;i++){
        snprintf(linea, sizeof(linea), "%d,%s,%d,%s,%.1f,%.1f,%.1f,%.1f,%.1f,%s,%.1f,%.1f,%.1f,%.1f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f",
        stationArray[numero].numero,
        stationArray[numero].nombre,
        stationArray[numero].idLocalidad,
        stationArray[numero].dato[i].fecha,
        stationArray[numero].dato[i].temp,
        stationArray[numero].dato[i].humedad, 
        stationArray[numero].dato[i].ptoRocio,
        stationArray[numero].dato[i].precip,
        stationArray[numero].dato[i].velocViento,
        stationArray[numero].dato[i].direcViento,
        stationArray[numero].dato[i].rafagaMax,
        stationArray[numero].dato[i].presion,
        stationArray[numero].dato[i].radiacion,
        stationArray[numero].dato[i].tempSuelo1,
        stationArray[numero].dato[i].tempSuelo2,
        stationArray[numero].dato[i].tempSuelo3,
        stationArray[numero].dato[i].humedadSuelo1,
        stationArray[numero].dato[i].humedadSuelo2,
        stationArray[numero].dato[i].humedadSuelo3,
        stationArray[numero].dato[i].humedadHoja);
        // fprintf(stationFile, "%s\n",linea);
        printf("%s\n", linea);
        sendToSocket(newsockfd, linea);
    }
    //

    //envio /END y espero ack
	sendToSocket(newsockfd,"/FINISH");
	sendToSocket(newsockfd,endMsg);
    //

    //cierro el proceso
	return;//esta bien? ver $ ps
    //

    // int skipResult = 0;
    // char fileName[15];
    // snprintf(fileName, sizeof(fileName), "estacion%d.CSV", nroEstacion);
    // snprintf(fileName, sizeof(fileName), "estacion.CSV");

    // FILE* stationFile = fopen(fileName,"w");

    // if (stationFile == NULL) {
    //   perror("Error exporting data");
    //   exit(EXIT_FAILURE);
    // }    
    //////////////////////////////////////////////////////////////////////////
    //  si el sensor no esta, poner "--"
    //  ver de guardar archivo con nombre de estacion
    //////////////////////////////////////////////////////////////////////////
    // rewind(stream);
    // printLineToFile(stream, stationFile, 3);
    // char linea [350];
    // for(int i=0;i<stationArray[numero].cantElem;i++){
    //     snprintf(linea, sizeof(linea), "%d,%s,%d,%s,%.1f,%.1f,%.1f,%.1f,%.1f,%s,%.1f,%.1f,%.1f,%.1f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f",
    //     stationArray[numero].numero,
    //     stationArray[numero].nombre,
    //     stationArray[numero].idLocalidad,
    //     stationArray[numero].dato[i].fecha,
    //     stationArray[numero].dato[i].temp,
    //     stationArray[numero].dato[i].humedad, 
    //     stationArray[numero].dato[i].ptoRocio,
    //     stationArray[numero].dato[i].precip,
    //     stationArray[numero].dato[i].velocViento,
    //     stationArray[numero].dato[i].direcViento,
    //     stationArray[numero].dato[i].rafagaMax,
    //     stationArray[numero].dato[i].presion,
    //     stationArray[numero].dato[i].radiacion,
    //     stationArray[numero].dato[i].tempSuelo1,
    //     stationArray[numero].dato[i].tempSuelo2,
    //     stationArray[numero].dato[i].tempSuelo3,
    //     stationArray[numero].dato[i].humedadSuelo1,
    //     stationArray[numero].dato[i].humedadSuelo2,
    //     stationArray[numero].dato[i].humedadSuelo3,
    //     stationArray[numero].dato[i].humedadHoja);
    //     // fprintf(stationFile, "%s\n",linea);
    //     printf("%s\n", linea);
    // }

    // fclose(stationFile);
	//////
	// FILE * fp;

	// fp = fopen ("prueba1.CSV", "rb");
	// int res;
	// while((res = fread(buffer, sizeof(char), TAM, fp)) > 0){
	// 	printf("%s", buffer);
	// 	// sleep(1);
	// 	// printf("%d\n",res );
	// 	n = sendto( sockfd, (void *)buffer, TAM, 0, (struct sockaddr *)&dest_addr, tamano_direccion );
	// 	if ( n < 0 ) {
	// 		perror( "Escritura en socket" );
	// 		exit( 1 );
	// 	}
	// 	memset( buffer, 0, sizeof( buffer ) );
	// 	n = recvfrom( sockfd, (void *)buffer, TAM, 0, (struct sockaddr *)&dest_addr, &tamano_direccion );
	// 	if ( n < 0 ) {
	// 		perror( "Lectura de socket" );
	// 		exit( 1 );
	// 	}
	// 	if(strcmp(buffer, "/ACK") == 0){
	// 		//recibi ack, mando la linea que sigue
	// 	}
	// }

	// memset( buffer, 0, sizeof( buffer ) );
	// strcpy(buffer, "/END");
	// n = sendto( sockfd, (void *)buffer, TAM, 0, (struct sockaddr *)&dest_addr, tamano_direccion );
	// printf("\n");

	// //espero ack final
	// n = recvfrom( sockfd, (void *)buffer, TAM, 0, (struct sockaddr *)&dest_addr, &tamano_direccion );
	// if ( n < 0 ) {
	// 	perror( "Lectura de socket" );
	// 	exit( 1 );
	// }

	// fclose(fp);

	// /////
	return;
}

void
procesar_input(int newsockfd, struct Estacion stationArray[], char buffer[]){
	char **args;
	args = malloc(LSH_TOK_BUFSIZE * sizeof(char*));
	int numTokens;
	numTokens = split_line(buffer, args);
	
    printf("cantidad de tokens: %d\n",numTokens);

    if(numTokens == 0){
    	printf("mensaje vacio\n");
		sendToSocket(newsockfd,endMsg);
    }

    else if(!strcmp(args[0],"listar")){
    	if(numTokens != 1){
    		perror("listar");
    		argError(newsockfd);
    		return;
    	}
        listarEstaciones(stationArray, newsockfd);
    }

    else if(!strcmp(args[0],"desconectar")){
        if(numTokens != 1){
        	perror("desconectar");
        	argError(newsockfd);
        	return;
        }
        sendToSocket(newsockfd,disconnectMsg);
    	printf( "PROCESO %d. Como recibí 'fin', termino la ejecución.\n\n", getpid() );
		exit(0);
    }

    if(!strcmp(args[0],"descargar")){
        char caracter;
        int numero;
        strcpy(&caracter,args[1]);
        if(isdigit(caracter)){
            numero = atoi(&caracter);
            descargarEstacion(numero, stationArray, newsockfd);
        }
    }

    else if(!strcmp(args[0],"diario_precip")){
    	if(numTokens != 2){
    		perror("diario_precip");
    		argError(newsockfd);
    		return;
    	}
        char caracter;
        int numero;
        strcpy(&caracter,args[1]);
        if(isdigit(caracter)){
            numero = atoi(&caracter);
            diarioPrecipitacion(stationArray,numero,newsockfd);
        }
    }

    else if(!strcmp(args[0],"mensual_precip")){
    	if(numTokens != 2){
    		perror("mensual_precip");
    		argError(newsockfd);
    		return;
    	}
        char caracter;
        int numero;
        strcpy(&caracter,args[1]);
        if(isdigit(caracter)){
            numero = atoi(&caracter);
            mensualPrecipitacion(stationArray, numero, newsockfd);
        }
    }

    else if(!strcmp(args[0],"ayuda")){
    	if(numTokens != 1){
    		perror("ayuda");
    		argError(newsockfd);
    		return;
    	}
    	showHelp(newsockfd);
    }

    else if(!strcmp(args[0],"promedio")){
    	if(numTokens != 2){
    		perror("promedio");
    		argError(newsockfd);
    		return;
    	}
        int result = promediar(stationArray,args[1], newsockfd);
        if(result < 0){
        	sendToSocket(newsockfd, "Sensor inexistente");
        	sendToSocket(newsockfd, endMsg);
        }
    }
	else{
		perror("else");
		argError(newsockfd);
	}
	free(args);
	memset( buffer, 0, TAM );
}

int 
main( int argc, char *argv[] ) {
	int sockfd, newsockfd, pid, status = 1, socketResult;
	char buffer[TAM];
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;

	startServer(&sockfd, &clilen, &serv_addr, &cli_addr);

	FILE* stream = fopen("../datos_meteorologicos.CSV", "r");
    // FILE* stream = fopen("../prueba1.CSV", "r");
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
            if(idEstacion != stationArray[j].numero){
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

        result = sscanf(line2, "%*d,%*[^','],%*d,%[^','],%f,%f,%f"
                                   ",%f,%f,%[^','],%f,%f,%f,%f,%f"
                                   ",%f,%f,%f,%f,%f",
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
            //si estoy llenando la primer fila, guardo nombre de estacion
            //y datos una sola vez
            result = sscanf(line2, "%d,%[^','],%d",
                                    &stationArray[j].numero,
                                    stationArray[j].nombre,
                                    &stationArray[j].idLocalidad);
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

			socketResult = sendToSocket(newsockfd, welcome_message);

			while ( 1 ) {
				socketResult = readFromSocket(newsockfd, buffer);
				printf( "PROCESO %d. ", getpid() );
				printf("Recibí: %*.*s\n", socketResult, socketResult, buffer);
				procesar_input(newsockfd, stationArray, buffer);
			}
		}
		else {
			printf( "SERVIDOR: Nuevo cliente, que atiende el proceso hijo: %d\n", pid );
			close( newsockfd );
		}
	}
	return 0; 
} 

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

void
argError(int newsockfd){
	sendToSocket(newsockfd, argErrorMsg);
	sendToSocket(newsockfd,endMsg);
}