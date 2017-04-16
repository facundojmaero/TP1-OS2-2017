#include "../include/comunes.h"
#include "../include/funciones_servidor_cc.h"

int 
main( int argc, char *argv[] ) {
    int sockfd, newsockfd, pid, status = 1, socketResult;
    char buffer[TAM];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    start_server(&sockfd, &clilen, &serv_addr, &cli_addr);

    FILE* stream = fopen("../datos_meteorologicos.CSV", "r");
    if (stream == NULL) {
      perror("Error opening CSV table");
      exit(EXIT_FAILURE);
    }

    size_t characters = 0;
    size_t len = 0;
    char* line2 = NULL;
    char* nombreColumnas = NULL;

    const char s[2] = ",";
    char *token;
    struct Estacion stationArray[10];
    int i=0;//fila del archivo
    int j=0;//numero de estacion
    int idEstacion;

    skip_lines(stream,INICIO_ESTACIONES-1);
    characters = getline(&nombreColumnas,&len,stream);
    struct sensor_disponible sensores_temp[16];

    int cuenta=0;
    token = strtok(nombreColumnas, s);
    while( token != NULL ) {
        if(cuenta>3){
        strcpy(sensores_temp[cuenta-4].nombreSensor,token);
        }
        token = strtok(NULL, s);
        cuenta++;
    }

    while((characters = getline(&line2,&len,stream)) != -1 ){     
        
        sscanf(line2,"%d",&idEstacion);
        if (i != 0){
            //Si no estoy llenando la primer fila
            //Comparo token (id de estacion) con id de estacion anterior para
            //ver si pase a otra estacion
            if(idEstacion != stationArray[j].numero){
                //Si el ID es distinto al anterior, 
                //termine de guardar una estacion y paso a la siguiente
                //Guardo el tamaño de la estacion (cant lineas)
                stationArray[j].cantElem = i;
                j++;
                i=0;
                if(j==5){break;}
            }
        }

        sscanf(line2, "%*d,%*[^','],%*d,%[^','],%f,%f,%f"
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
        if(i==0){
            check_sensores(stationArray, j, line2,sensores_temp);
            //si estoy llenando la primer fila, guardo nombre de estacion
            //y datos una sola vez
            sscanf(line2, "%d,%[^','],%d",
                                    &stationArray[j].numero,
                                    stationArray[j].nombre,
                                    &stationArray[j].idLocalidad);
        }
        i++;

    //Guardo la cantidad de elementos de la ultima estacion
    stationArray[j].cantElem = i;
    }

    //acepto conexiones
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
                socketResult = read_from_socket(newsockfd,buffer);

                printf( "PROCESO %d. ", getpid() );
                printf("Recibí: %*.*s\n", socketResult, socketResult, buffer);

                char user[20];
                char password[20];

                sscanf(buffer,"%s%s",user,password);

                if(strcmp(user,correct_user) || strcmp(password,correct_pw)){
                    //no son iguales
                    socketResult = send_to_socket(newsockfd, errormsg);
                    printf("\nwrong password\n");
                    close(newsockfd);
                    exit(0);
                }
                else{
                    socketResult = send_to_socket(newsockfd,okmsg);
                    printf("\ncorrect password\n");
                    status=0;
                }
            }

            socketResult = send_to_socket(newsockfd, welcome_message);

            while ( 1 ) {
                socketResult = read_from_socket(newsockfd, buffer);
                printf( "PROCESO %d. ", getpid() );
                printf("Recibí: %*.*s\n", socketResult, socketResult, buffer);
                procesar_input(newsockfd, stationArray, buffer, stream);
            }
        }
        else {
            printf( "SERVIDOR: Nuevo cliente, que atiende el proceso hijo: %d\n", pid );
            close( newsockfd );
        }
    }
    fclose(stream);
    return 0; 
}

int 
initialize_udp_client_with_args(
    socklen_t *tamano_direccion, 
    struct sockaddr_in* dest_addr
    ){
    
    int sockudp, puerto;
    struct hostent *server;

    char *argv[] = {"localhost","6020"};

    server = gethostbyname( argv[0] );
    if ( server == NULL ) {
        fprintf( stderr, "ERROR, no existe el host\n");
        exit(0);
    }

    puerto = atoi( argv[1] );
    sockudp = socket( AF_INET, SOCK_DGRAM, 0 );
    if (sockudp < 0) {
        perror( "apertura de socket" );
        exit( 1 );
    }

    dest_addr->sin_family = AF_INET;
    dest_addr->sin_port = htons( puerto );
    dest_addr->sin_addr = *( (struct in_addr *)server->h_addr );
    memset( (dest_addr->sin_zero), '\0', 8 );

    *tamano_direccion = sizeof( *dest_addr );
    return sockudp;
}

size_t
get_variable_offset(char variable[], int* indiceSensor){

    if(strcmp(variable, "temp") == 0){
        *indiceSensor = 0;
        return (offsetof(struct dato_estacion, temp));
    }
    else if(strcmp(variable, "humedad") == 0){
        *indiceSensor = 1;
        return (offsetof(struct dato_estacion, humedad));
    }
    else if(strcmp(variable, "ptoRocio") == 0){
        *indiceSensor = 2;
        return (offsetof(struct dato_estacion, ptoRocio));
    }
    else if(strcmp(variable, "precip") == 0){
        *indiceSensor = 3;
        return (offsetof(struct dato_estacion, precip));
    }
    else if(strcmp(variable, "velocViento") == 0){
        *indiceSensor = 4;
        return (offsetof(struct dato_estacion, velocViento));
    }
    else if(strcmp(variable, "rafagaMax") == 0){
        *indiceSensor = 6;
        return (offsetof(struct dato_estacion, rafagaMax));
    }
    else if(strcmp(variable, "presion") == 0){
        *indiceSensor = 7;
        return (offsetof(struct dato_estacion, presion));
    }
    else if(strcmp(variable, "radiacion") == 0){
        *indiceSensor = 8;
        return (offsetof(struct dato_estacion, radiacion));
    }
    else if(strcmp(variable, "tempSuelo1") == 0){
        *indiceSensor = 9;
        return (offsetof(struct dato_estacion, tempSuelo1));
    }
    else if(strcmp(variable, "tempSuelo2") == 0){
        *indiceSensor = 10;
        return (offsetof(struct dato_estacion, tempSuelo2));
    }
    else if(strcmp(variable, "tempSuelo3") == 0){
        *indiceSensor = 11;
        return (offsetof(struct dato_estacion, tempSuelo3));
    }
    else if(strcmp(variable, "humedadSuelo1") == 0){
        *indiceSensor = 12;
        return (offsetof(struct dato_estacion, humedadSuelo1));
    }
    else if(strcmp(variable, "humedadSuelo2") == 0){
        *indiceSensor = 13;
        return (offsetof(struct dato_estacion, humedadSuelo2));
    }
    else if(strcmp(variable, "humedadSuelo3") == 0){
        *indiceSensor = 14;
        return (offsetof(struct dato_estacion, humedadSuelo3));
    }
    else if(strcmp(variable, "humedadHoja") == 0){
        *indiceSensor = 15;
        return (offsetof(struct dato_estacion, humedadHoja));
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
    offset = get_variable_offset(variable, &indiceSensor);
    if(offset > longitudElemento*4){
        printf("variable inexistente\n");
        return -1;
    }

    printf("Promedios variable "BOLDGREEN"%s:"RESET"\n", variable);
    snprintf(mensaje, TAM, "Promedios variable "BOLDGREEN"%s:"RESET"\n", variable);
    send_to_socket(newsockfd, mensaje);

    for(int j=0; j < NRO_ESTACIONES; j++){
        if(stationArray[j].sensores[indiceSensor].esta == 0){
            printf("%d - %s: No se encuentran datos.\n",
                stationArray[j].numero, stationArray[j].nombre);
            snprintf(mensaje,TAM, "%d - %s: "BOLDRED"No se encuentran datos."RESET,
                stationArray[j].numero, stationArray[j].nombre);
            send_to_socket(newsockfd, mensaje);
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
        printf("%d - %s: %.1f\n", stationArray[j].numero, 
            stationArray[j].nombre,suma);
        snprintf(mensaje, TAM, "%d - %s: %.1f", stationArray[j].numero, 
            stationArray[j].nombre,suma);;
        send_to_socket(newsockfd, mensaje);
    }
    send_to_socket(newsockfd, endMsg);
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
		send_to_socket(newsockfd, "Numero de estacion inexistente");
    	send_to_socket(newsockfd, endMsg);
		return;
	}

    float precipAcumulada=0;
    char mensaje[TAM];
    int i;
    printf("Precipitacion acumulada por dia.\n%d - Estacion %s:\n",
    	estaciones[nro].numero, estaciones[nro].nombre);
    snprintf(mensaje, TAM, "Precipitacion acumulada por dia.\n%d - Estacion %s:",
        estaciones[nro].numero, estaciones[nro].nombre);
    send_to_socket(newsockfd, mensaje);
    for (i = 0; i < estaciones[nro].cantElem; ++i)
    {
        if(i!=0 && (strcmp(estaciones[nro].dato[i].dia, estaciones[nro].dato[i-1].dia))){
            //termine de recorrer un dia. Sumo todo, muestro y reseteo contador
            printf("	%s: %.1f mm\n", estaciones[nro].dato[i-1].dia, precipAcumulada);
            snprintf(mensaje, TAM, "	%s: %.1f mm", 
                estaciones[nro].dato[i-1].dia, precipAcumulada);
    		send_to_socket(newsockfd, mensaje);
            precipAcumulada = 0;
        }
        precipAcumulada+=estaciones[nro].dato[i].precip;
    }
    printf("	%s: %.1f mm\n", estaciones[nro].dato[i-1].dia, precipAcumulada);
    snprintf(mensaje, TAM, "	%s: %.1f mm", 
        estaciones[nro].dato[i-1].dia, precipAcumulada);
    send_to_socket(newsockfd, mensaje);
    send_to_socket(newsockfd, endMsg);
}

void 
mensual_precip(struct Estacion estaciones[], int nro, int newsockfd){	
	if(check_estacion_existente(estaciones, &nro) == 0){
		send_to_socket(newsockfd, "Numero de estacion inexistente");
    	send_to_socket(newsockfd, endMsg);
		return;
	}
	float precipAcumulada=0;

    for (int i = 0; i < estaciones[nro].cantElem; ++i)
    {
        precipAcumulada+=estaciones[nro].dato[i].precip;
    }
    printf("Precipitacion acumulada mensual.\n%d - Estacion %s: %.1f mm\n", 
        estaciones[nro].numero,estaciones[nro].nombre,precipAcumulada);
    char mensaje[TAM];

    snprintf(mensaje, TAM, "Precipitacion acumulada mensual.\n%d - ""Estacion %s: %.1f mm\n", 
        estaciones[nro].numero,estaciones[nro].nombre,precipAcumulada);
	send_to_socket(newsockfd, mensaje);
    send_to_socket(newsockfd, endMsg);
}

void 
listar(struct Estacion estaciones[], int newsockfd){
    char mensaje[TAM];
    printf("Estaciones disponibles:\n\n");
    snprintf(mensaje, TAM, "Estaciones disponibles:\n");
    send_to_socket(newsockfd, mensaje);
    for (int i = 0; i < NRO_ESTACIONES; ++i)
    {
        printf("%d - %s. Sensores con datos:\n",
            estaciones[i].numero,estaciones[i].nombre);
        snprintf(mensaje,TAM,"%d -  %s. Sensores con datos:",
            estaciones[i].numero,estaciones[i].nombre);
        send_to_socket(newsockfd, mensaje);
        for (int j = 0; j < NRO_SENSORES; ++j)
        {
            if(estaciones[i].sensores[j].esta){
                printf("    %s\n", estaciones[i].sensores[j].nombreSensor);
                snprintf(mensaje,TAM,"    %s", estaciones[i].sensores[j].nombreSensor);
                send_to_socket(newsockfd, mensaje);
            }
        }
        printf("\n\n");
        snprintf(mensaje, TAM, "\n");
        send_to_socket(newsockfd, mensaje);
    }
    send_to_socket(newsockfd, endMsg);
}

void 
show_help(int newsockfd){
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

	send_to_socket(newsockfd, mensaje);
	send_to_socket(newsockfd, endMsg);
}

void 
check_sensores(struct Estacion stationArray[], int j, char* line2, 
    struct sensor_disponible sensores_temp[] ){
    char* tempstr = calloc(strlen(line2)+1, sizeof(char));
    strcpy(tempstr, line2);
    const char s[2] = ",";
    char *token;
    int cuenta=0, skip=0;

    for (int i = 0; i < NRO_SENSORES; ++i)
    {
        strcpy(stationArray[j].sensores[i].nombreSensor,sensores_temp[i].nombreSensor);
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
descargar_estacion(int numero, struct Estacion stationArray[], int newsockfd, FILE* stream){
    if(check_estacion_existente(stationArray, &numero) == 0){
        printf("estacion inexistente, numero %d\n",numero );
        send_to_socket(newsockfd, "Numero de estacion inexistente");
        send_to_socket(newsockfd, endMsg);
        return;
    }

    //envio aviso inicio de transferencia a cliente
    send_to_socket(newsockfd, start_UDP_Msg);
    char buffer[TAM];

    //aca levanto el cliente UDP (se invierten los roles)
    //espero que el servidor udp este levantado
    read_from_socket(newsockfd, buffer);
    if(strcmp(buffer, udp_ready) != 0){
        perror("descargar_estacion: udp ready");
        return;
    }
    printf("udp ready: %s\n", buffer);
    //

    //inicializo el cliente udp
    socklen_t tamano_direccion;
    int sockudp;
    struct sockaddr_in dest_addr;

    sockudp = initialize_udp_client_with_args(&tamano_direccion , &dest_addr);

    //envio filename y espero ack
    // send_to_socket(newsockfd,"descarga.txt");
    send_udp(sockudp, "descarga.txt", &dest_addr, tamano_direccion);
	// read_from_socket(newsockfd, buffer); //leo el ack
    recv_udp(sockudp, buffer, &dest_addr, &tamano_direccion);
    if(strcmp(buffer, ack_msg) != 0){
        perror("descargar_estacion: ack invalido");
        return;
    }
    //

    memset( buffer, 0, sizeof( buffer ) );

    //envio cabecera de archivo
    size_t len = 0;
    char* cabecera = NULL;
    rewind(stream);
    for (int i = 0; i < 3; ++i)
    {
        getline(&cabecera,&len,stream);
        send_udp(sockudp, cabecera, &dest_addr, tamano_direccion);
        printf("BUFFER-> %s", buffer);
        recv_udp(sockudp, buffer, &dest_addr, &tamano_direccion);
        if(strcmp(buffer, ack_msg) != 0){
            perror("descargar_estacion: ack invalido");
            close(sockudp);
            return;
        }
    }
    //

    //envio lineas mientras haya datos para enviar, y espero ack cada vez
    for(int i=0;i<stationArray[numero].cantElem;i++){
        snprintf(buffer, sizeof(buffer), 
        "%d,%s,%d,%s,%.1f,%.1f,%.1f,%.1f,%.1f,%s,%.1f,%.1f,%.1f,%.1f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f",
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

        // send_to_socket(newsockfd, buffer);
        send_udp(sockudp, buffer, &dest_addr, tamano_direccion);
        printf("BUFFER-> %s\n", buffer);
        // read_from_socket(newsockfd, buffer); //leo el ack
        recv_udp(sockudp, buffer, &dest_addr, &tamano_direccion);
        if(strcmp(buffer, ack_msg) != 0){
            perror("descargar_estacion: ack invalido");
            close(sockudp);
            return;
        }
    }
    //

    //envio /END y espero ack
	// send_to_socket(newsockfd,end_UDP_Msg);
    send_udp(sockudp, end_UDP_Msg, &dest_addr, tamano_direccion);
    // read_from_socket(newsockfd, buffer); //leo el ack
    recv_udp(sockudp, buffer, &dest_addr, &tamano_direccion);
    if(strcmp(buffer, ack_msg) != 0){
        perror("descargar_estacion: ack invalido");
        close(sockudp);
        return;
    }
	send_to_socket(newsockfd,endMsg);
    //
    printf("fin de funcion\n");
    //cierro el proceso
    close(sockudp);
	return;
}

void
procesar_input(int newsockfd, struct Estacion stationArray[], char buffer[], FILE* stream){
	char **args;
	args = malloc(LSH_TOK_BUFSIZE * sizeof(char*));
	int numTokens;
	numTokens = split_line(buffer, args);
	
    printf("cantidad de tokens: %d\n",numTokens);

    if(numTokens == 0){
    	printf("mensaje vacio\n");
		send_to_socket(newsockfd,endMsg);
    }

    else if(!strcmp(args[0],"listar")){
    	if(numTokens != 1){
    		perror("listar");
    		arg_error(newsockfd);
    		return;
    	}
        listar(stationArray, newsockfd);
    }

    else if(!strcmp(args[0],"desconectar")){
        if(numTokens != 1){
        	perror("desconectar");
        	arg_error(newsockfd);
        	return;
        }
        send_to_socket(newsockfd,disconnectMsg);
    	printf( "PROCESO %d. Como recibí 'fin', termino la ejecución.\n\n", getpid() );
		exit(0);
    }

    else if(!strcmp(args[0],"descargar")){
        if(numTokens != 2){
            perror("diario_precip");
            arg_error(newsockfd);
            return;
        }
        char caracter;
        strcpy(&caracter,args[1]);
        if(isdigit(caracter)){
            int numero = atoi(&caracter);
            descargar_estacion(numero, stationArray, newsockfd, stream);
        }
        else{
            arg_error(newsockfd);
        }
    }

    else if(!strcmp(args[0],"diario_precip")){
    	if(numTokens != 2){
    		perror("diario_precip");
    		arg_error(newsockfd);
    		return;
    	}
        char caracter;
        strcpy(&caracter,args[1]);
        if(isdigit(caracter)){
            int numero = atoi(&caracter);
            diarioPrecipitacion(stationArray,numero,newsockfd);
        }
        else{
            arg_error(newsockfd);
        }
    }

    else if(!strcmp(args[0],"mensual_precip")){
    	if(numTokens != 2){
    		perror("mensual_precip");
    		arg_error(newsockfd);
    		return;
    	}
        char caracter;
        strcpy(&caracter,args[1]);
        if(isdigit(caracter)){
            int numero = atoi(&caracter);
            mensual_precip(stationArray, numero, newsockfd);
        }
        else{
            arg_error(newsockfd);
        }
    }

    else if(!strcmp(args[0],"ayuda")){
    	if(numTokens != 1){
    		perror("ayuda");
    		arg_error(newsockfd);
    		return;
    	}
    	show_help(newsockfd);
    }

    else if(!strcmp(args[0],"promedio")){
    	if(numTokens != 2){
    		perror("promedio");
    		arg_error(newsockfd);
    		return;
    	}
        int result = promediar(stationArray,args[1], newsockfd);
        if(result < 0){
        	send_to_socket(newsockfd, "Sensor inexistente");
        	send_to_socket(newsockfd, endMsg);
        }
    }
	else{
		arg_error(newsockfd);
	}
	free(args);
	memset( buffer, 0, TAM );
} 

void
arg_error(int newsockfd){
	send_to_socket(newsockfd, arg_errorMsg);
	send_to_socket(newsockfd,endMsg);
}