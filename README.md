# TP1-OS2-2017 

### Trabajo Práctico 1 - Sockets 
![Icono](http://www.drkappa.net/wp-content/uploads/2015/02/wsiconblog.png)
___
|Autor                               |
|------------------------------------|
|Maero Facundo - 38479441              |

## 1. Descripción del trabajo
---
Este proyecto permite acceder, controlar y transferir datos telemétricos entre el cliente y un servidor, que recopila logs provenientes de diferentes estaciones sensoras. Provee diversas funciones para la visualización de los datos, y una interfaz por línea de comandos.

## 2. Instalación
---
Los siguientes son los comandos útiles para el uso de la aplicación:

```$ make```              --> Compila el proyecto y genera ejecutable.
 
```$ make clean ```       --> Limpia para una nueva compilación.

```$ make cppcheck```     --> Ejecuta el programa CppCheck sobre el proyecto. (Debe estar instalado).

```$ ./sock_cli_i_cc```   --> Ejecuta el programa cliente.

```$ ./sock_srv_i_cc```   --> Ejecuta el programa servidor.

## 3. Ejecución
--- 
Para ejecutar el programa servidor, utilizar el comando 
```$ ./sock_srv_i_cc```
 Para ejecutar el programa cliente, utilizar el comando 
 ```$ ./sock_cli_c_cc```
 Para acceder al servidor desde el cliente, utilizar la IP del servidor con el comando: 
 ```$ facundo@IP_Servidor:6020```
  
 La contraseña de acceso es "alfajor". Para solicitar ayuda, escriba "ayuda".

## 4. CppCheck
--- 
Al compilar y linkear, se genera un archivo donde se guardan los posibles errores y advertencias que encuentre el programa CppCheck al realizar el análisis estático del código. Este archivo se encuentra en:
```
TP1-OS2-2017/err.txt
```
Si desea más información, remítase a la documentación proporcionada, que se encuentra en la ruta ```doc/html/index.html```
