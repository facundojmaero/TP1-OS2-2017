/**
* @mainpage main
* PROYECTO WEATHER STATION
* ===================
*
* @author Facundo Maero (facundojmaero@gmail.com)
* @date Abril, 2017
* @version V1.0
*
* @brief Esta es una aplicacion que  permite acceder a datos telemétricos en estaciones climáticas. Lenguaje: C.
* 
* @par INTRODUCCIÓN:
* Este proyecto permite acceder, controlar y transferir datos telemétricos entre el cliente
* y un servidor, que recopila logs provenientes de diferentes estaciones sensoras.
* Provee diversas funciones para la visualización de los datos, y una interfaz por 
* línea de comandos.
*
* @par INSTALACIÓN:
* Los siguientes son los comandos útiles para el uso de la aplicación:\n
*
* 	"make"				--> Compila el proyecto y genera ejecutable.
* 	"make clean"		--> Limpia para una nueva compilación.
* 	"make cppcheck" 	--> Ejecuta el programa CppCheck sobre el proyecto. (Debe estar instalado).
* 	"./sock_cli_i_cc"	--> Ejecuta el programa cliente.
* 	"./sock_srv_i_cc"	--> Ejecuta el programa servidor.
*
* @par EJECUCIÓN:
* Para ejecutar el programa servidor, utilizar el comando "./sock_srv_i_cc".
* Para ejecutar el programa cliente, utilizar el comando "./sock_cli_c_cc".
* Para acceder al servidor desde el cliente, utilizar la IP del servidor con el comando:
* facundo@<IP_Servidor>:6020.
* La contraseña de acceso es "alfajor".
* Para solicitar ayuda, escriba "ayuda".
*
*
*/