# Lab-3-Grupo-4-Redes
Integrantes:
Emmanuel Blanco – 202312743 
Tomás Torres – 202313980

# Documentación archivos de red TCP

> **Nota general:** las librerías marcadas como "estándar de C" son parte del lenguaje mismo (definidas por el estándar ISO C). Las marcadas como "POSIX" son específicas de sistemas tipo Unix (Linux, macOS) y son las que hacen posible la programación de sockets de bajo nivel que usamos en este laboratorio.

## **broker_tcp.c**

Librerías utilizadas:

- `#include <stdio.h>` **(estándar de C)**: Esta librería importa las funciones que usamos para imprimir mensajes por consola, sean mensajes de los publicadores o de la conexión de clientes, o mensajes de error. (De este módulo usamos `printf()` y `perror()`)

- `#include <stdlib.h>` **(estándar de C)**: De esta librería en específico usamos `exit()` para terminar el programa en caso de falla.

- `#include <sys/socket.h>` **(POSIX, no estándar de C)**: Esta librería contiene las funciones para declarar sockets de bajo nivel del sistema operativo. Gracias a esta librería podemos declarar los sockets junto con su protocolo de transporte y hacer la asociación de los sockets con la IP de la máquina donde se corre el programa del broker.

- `#include <arpa/inet.h>` **(POSIX, no estándar de C)**: De esta librería usamos funciones para la traducción de direcciones IP, como `htons()` (conversión del puerto a formato de red) y la estructura `sockaddr_in`.

- `#include <string.h>` **(estándar de C)**: Librería para manipulación de strings y bloques de memoria en general. La usamos para `memset()` (inicializar estructuras en cero antes de llenarlas), `strlen()` (medir la longitud de un mensaje), `strcmp()`/`strncmp()` (comparar strings, por ejemplo para reconocer el rol de algún cliente que se conecte al broker con `"PUB"` o `"SUB"`), `strchr()` (buscar el carácter `:` en los mensajes de publicación) y `strncpy()` (copiar el nombre del equipo con un límite de tamaño, evitando desbordar el buffer).

- `#include <strings.h>` **(POSIX, no estándar de C)**: Distinta de `<string.h>` (con "s" al final). Esta librería nos permite que strings escritos con mayúsculas o minúsculas sean reconocidos como iguales (`strcasecmp()`), para poder reconocer los tópicos a los que los suscriptores se suscriben independientemente de cómo haya sido su entrada.

- `#include <unistd.h>` **(POSIX, no estándar de C)**: De esta librería usamos la función `close()` para cerrar los sockets correctamente.

- `#include <sys/select.h>` **(POSIX, no estándar de C)**: Esta librería nos permite el manejo de múltiples sockets simultáneamente (multiplexado de E/S), necesario para que el broker atienda a los múltiples publicadores y suscriptores que se conecten, sin bloquearse esperando a uno solo. Trae la función `select()`, el tipo `fd_set`, y las macros `FD_ZERO()`, `FD_SET()`, `FD_ISSET()`.

**¿Cómo correr el programa?**
1. Compilar el archivo `gcc -Wall -Wextra -g -o brokertcp broker_tcp.c`
2. Correr usando `.\brokertcp`

## **publicador_tcp.c** 

Librerías utilizadas:

- `#include <stdio.h>` **(estándar de C)**: La usamos para `printf()` (mensajes informativos y el prompt de entrada), `fprintf()` (mensajes de error dirigidos a `stderr`), `perror()` (errores de syscalls), `fgets()` (leer líneas desde el teclado) y `snprintf()` (armar el mensaje de suscripción/publicación con formato).

- `#include <stdlib.h>` **(estándar de C)**: La usamos para `exit()` (terminar el programa ante una falla) y `atoi()` ("ASCII to integer", convierte el puerto recibido como texto por línea de comandos a un número entero).

- `#include <string.h>` **(estándar de C)**: La usamos para `memset()` (inicializar la estructura de dirección), `strlen()` (medir los mensajes antes de enviarlos), `strncmp()` (reconocer el comando `"salir"`) y `strchr()` (validar que el mensaje tenga el formato `equipo:mensaje`).

- `#include <sys/socket.h>` **(POSIX, no estándar de C)**: La usamos para `socket()` (crear el socket) y `send()` (enviar los mensajes de registro y las noticias al broker).

- `#include <arpa/inet.h>` **(POSIX, no estándar de C)**: La usamos para la estructura `struct sockaddr_in`, la función `inet_pton()` (convertir la IP de texto a formato binario de red) y `htons()` (convertir el puerto a formato de red).

**¿Cómo correr el programa?**
1. Compilar el archivo `gcc -Wall -Wextra -g -o pubtcp publisher_tcp.c`
2. Correr usando `.\pubtcp <ip_broker> 9000` Donde ip_broker corresponde a la dirección IP de la máquina donde ejecute el broker
3. Una vez ejecutado el programa seguir la estructura **TOPICO:***mensaje* para que los mensajes lleguen correctamente a los suscriptores

## **suscriptor_tcp.c**

Librerías utilizadas:

- `#include <stdio.h>` **(estándar de C)**: La usamos para `printf()` (mensajes informativos y las noticias recibidas), `perror()` (errores de syscalls) y `snprintf()` (armar el mensaje de suscripción `SUB <equipo>`).

- `#include <stdlib.h>` **(estándar de C)**: La usamos para `exit()` (terminar el programa ante una falla) y `atoi()` (convertir el puerto recibido por línea de comandos a número entero).

- `#include <string.h>` **(estándar de C)**: La usamos para `memset()` (inicializar la estructura de dirección) y `strlen()` (medir el mensaje de registro antes de enviarlo).

- `#include <sys/socket.h>` **(POSIX, no estándar de C)**: La usamos para `socket()` (crear el socket), `send()` (enviar el mensaje de suscripción) y `recv()` (recibir las noticias que el broker reenvía).

- `#include <arpa/inet.h>` **(POSIX, no estándar de C)**: La usamos para `struct sockaddr_in`, `inet_pton()` y `htons()`, igual que en el publicador.

**¿Cómo correr el programa?**
1. Compilar el archivo `gcc -Wall -Wextra -g -o subtcp subscriber_tcp.c`
2. Correr usando `.\subtcp <ip_broker> 9000 <partido>` Al igual que con el publicador se debe ingresar la IP del broker, más el tópico al que se quiere suscribir (Nota:El programa también recibe ALL como tópico para recibir noticias de todos los partidos)
3. Una vez ejecutado el programa esperar a recibir mensajes del broker (Nota: Importante que desde el publicador el tópico se escriba correctamente o no van a recibirse los mensajes, no pueden haber errores)

## **UDP:**

**Librerías utilizadas:**

## **broker_udp.c**

#`include <stdio.h>` **(estándar de C)**: Esta librería importa las funciones de entrada y salida estándar. Se utiliza para imprimir mensajes por consola (mensajes informativos, notificaciones de eventos o avisos de error) y para la lectura/formateo de datos. (De este módulo se usan funciones como printf(), fprintf(), perror(), fgets() y snprintf()).

#`include <stdlib.h>` **(estándar de C):** Librería de utilidades generales del sistema. Se utiliza para la gestión de terminación del programa y conversión de tipos de datos. (De este módulo se usan funciones como exit() para finalizar la ejecución ante fallas críticas y atoi() para convertir cadenas de texto a enteros, por ejemplo al procesar el puerto pasado por línea de comandos).

#`include <string.h>` **(estándar de C):** Librería para la manipulación de cadenas de caracteres y bloques de memoria. (Se utiliza para memset() para limpiar o inicializar estructuras en cero, strlen() para medir la longitud de los mensajes enviadas o recibidos, strcmp()/strncmp() para comparar comandos como "PUB", "SUB" o "salir", strchr() para buscar delimitadores como el carácter : y strncpy() para realizar copias seguras de texto evitando desbordamientos de buffer).

#`include <unistd.h>` **(POSIX, no estándar de C):** Proporciona acceso a la API de llamadas al sistema del estándar POSIX. (De esta librería se utiliza principalmente la función close() para cerrar de manera limpia y adecuada los sockets y descriptores de archivo abiertos).

#`include <sys/socket.h>` **(POSIX, no estándar de C):** Cabecera principal para el manejo de sockets en sistemas UNIX/Linux. Define las estructuras de datos y las llamadas a sistema necesarias para la comunicación por red. (Se utiliza para la creación de sockets con socket(), el envío de datos con send() o sendto(), la recepción con recv() o recvfrom(), así como la vinculación de puertos mediante bind()).

#`include <arpa/inet.h>` **(POSIX, no estándar de C):** Librería encargada de las operaciones con direcciones de Internet y conversión de formatos entre la representación de la máquina (host) y la red (network byte order). (De este módulo se utilizan funciones como htons() para convertir el número de puerto a formato de red, inet_pton() para convertir direcciones IP en formato texto a su representación binaria y la estructura struct sockaddr_in).

#`include <netinet/in.h>` **(POSIX, no estándar de C):** Define las constantes y estructuras fundamentales para la familia de protocolos de Internet (IPv4 / IPv6). Complementa a `<arpa/inet.h>` definiendo constantes como INADDR_ANY (para vincular el socket a todas las interfaces locales de red) y tipos de datos como in_port_t, in_addr_t y la estructura struct sockaddr_in.

#`include <strings.h>` **(POSIX, no estándar de C):** Cabecera histórica de POSIX `(distinta de <string.h>).` Proporciona funciones para la comparación de cadenas insensibles a mayúsculas y minúsculas. (Se utiliza para strcasecmp(), permitiendo que el programa reconozca nombres de equipos o tópicos de suscripción de manera equivalente independientemente de si el usuario los escribe en mayúsculas o minúsculas).

## **publisher_udp.c**

#`include <stdio.h>` **(estándar de C):** Esta librería importa las funciones de entrada y salida estándar. Se utiliza para imprimir mensajes por consola (mensajes informativos, notificaciones de eventos o avisos de error) y para la lectura/formateo de datos. (De este módulo se usan funciones como printf(), fprintf(), perror(), fgets() y snprintf()).

#`include <stdlib.h>` **(estándar de C):** Librería de utilidades generales del sistema. Se utiliza para la gestión de terminación del programa y conversión de tipos de datos. (De este módulo se usan funciones como exit() para finalizar la ejecución ante fallas críticas y atoi() para convertir cadenas de texto a enteros, por ejemplo al procesar el puerto pasado por línea de comandos).

#`include <string.h>` **(estándar de C):** Librería para la manipulación de cadenas de caracteres y bloques de memoria. (Se utiliza para memset() para limpiar o inicializar estructuras en cero, strlen() para medir la longitud de los mensajes enviadas o recibidos, strcmp()/strncmp() para comparar comandos como "PUB", "SUB" o "salir", strchr() para buscar delimitadores como el carácter : y strncpy() para realizar copias seguras de texto evitando desbordamientos de buffer).

#`include <unistd.h>` **(POSIX, no estándar de C):** Proporciona acceso a la API de llamadas al sistema del estándar POSIX. (De esta librería se utiliza principalmente la función close() para cerrar de manera limpia y adecuada los sockets y descriptores de archivo abiertos).

#`include <sys/socket.h>` **(POSIX, no estándar de C):** Cabecera principal para el manejo de sockets en sistemas UNIX/Linux. Define las estructuras de datos y las llamadas a sistema necesarias para la comunicación por red. (Se utiliza para la creación de sockets con socket(), el envío de datos con send() o sendto(), la recepción con recv() o recvfrom(), así como la vinculación de puertos mediante bind()).

#`include <arpa/inet.h>` **(POSIX, no estándar de C):** Librería encargada de las operaciones con direcciones de Internet y conversión de formatos entre la representación de la máquina (host) y la red (network byte order). (De este módulo se utilizan funciones como htons() para convertir el número de puerto a formato de red, inet_pton() para convertir direcciones IP en formato texto a su representación binaria y la estructura struct sockaddr_in).

#`include <netinet/in.h>` **(POSIX, no estándar de C):** Define las constantes y estructuras fundamentales para la familia de protocolos de Internet (IPv4 / IPv6). Complementa a `<arpa/inet.h>` definiendo constantes como INADDR_ANY (para vincular el socket a todas las interfaces locales de red) y tipos de datos como in_port_t, in_addr_t y la estructura struct sockaddr_in.

## **subscriber_udp.c**

#`include <stdio.h>` **(estándar de C):** Esta librería importa las funciones de entrada y salida estándar. Se utiliza para imprimir mensajes por consola (mensajes informativos, notificaciones de eventos o avisos de error) y para la lectura/formateo de datos. (De este módulo se usan funciones como printf(), fprintf(), perror(), fgets() y snprintf()).

#`include <stdlib.h>` **(estándar de C):** Librería de utilidades generales del sistema. Se utiliza para la gestión de terminación del programa y conversión de tipos de datos. (De este módulo se usan funciones como exit() para finalizar la ejecución ante fallas críticas y atoi() para convertir cadenas de texto a enteros, por ejemplo al procesar el puerto pasado por línea de comandos).

#`include <string.h>` **(estándar de C):** Librería para la manipulación de cadenas de caracteres y bloques de memoria. (Se utiliza para memset() para limpiar o inicializar estructuras en cero, strlen() para medir la longitud de los mensajes enviadas o recibidos, strcmp()/strncmp() para comparar comandos como "PUB", "SUB" o "salir", strchr() para buscar delimitadores como el carácter : y strncpy() para realizar copias seguras de texto evitando desbordamientos de buffer).

#`include <sys/socket.h>` **(POSIX, no estándar de C):** Cabecera principal para el manejo de sockets en sistemas UNIX/Linux. Define las estructuras de datos y las llamadas a sistema necesarias para la comunicación por red. (Se utiliza para la creación de sockets con socket(), el envío de datos con send() o sendto(), la recepción con recv() o recvfrom(), así como la vinculación de puertos mediante bind()).

#`include <arpa/inet.h>` **(POSIX, no estándar de C):** Librería encargada de las operaciones con direcciones de Internet y conversión de formatos entre la representación de la máquina (host) y la red (network byte order). (De este módulo se utilizan funciones como htons() para convertir el número de puerto a formato de red, inet_pton() para convertir direcciones IP en formato texto a su representación binaria y la estructura struct sockaddr_in).


**¿Cómo correr el programa?**

1. Compilar los tres archivos, del broker, subscriber y publisher, puede copiar los siguientes comandos para hacerlo:

**Comando broker:** "gcc -o broker_udp broker_udp.c"
**Comando publisher:** "gcc -o publisher_udp publisher_udp.c"
**comando subscriber:** "gcc -o subscriber_udp subscriber_udp.c"

2. Luego de eso, hace falta abrir una terminal por cada uno, una para el broker, y una para cada uno de los subscribers y publishers que se quieran utilizar. 

**Nota:** el puerto por defecto es el 9000, por lo que será el que se use en los ejemplos de cómo correr el programa. 

**Comando broker:** "./broker_udp 9000"

**Comando subscriber:** "./subscriber_udp 127.0.0.1 9000 ALL"

Tener en cuenta dos cosas, primero que se usa 127.0.0.1 porque es el localhost, y segundo que **ALL** sirve para recibir noticias de todos los partidos activos en ese momento, en caso de querer recibir noticias de un partido en particular se hace lo siguiente:

**Comando subscriber:** "./subscriber_udp 127.0.0.1 9000 Millonarios-SantaFe"

El nombre del partido es el nombre del primer equipo, y un guión medio, seguido del nombre del segundo equipo. No hace falta ejecutar el código del publisher antes, si se ejecuta antes el subscriber, se queda esperando a las noticias. 

**Comando publisher:** "./publisher_udp 127.0.0.1 9000"

3. En el publisher se mandan las noticias de los partidos con un comando Llave-Valor, el cual tiene la siguiente estructura.

**Mensaje publisher:** "Millonarios-SantaFe: Inicio del partido en el Campin"

La estructura es NombrePartido:NoticiaPartido. Recordar que el nombre del partido es Equipo1-Equipo2, por lo que el mensaje debería ser Equipo1-Equipo2:NoticiaPartido. Si el subscriber está suscrito a un partido en específico, solo recibirá los mensajes del publisher que tengan esos dos equipos en la llave del mensaje.

**Nota:** para dejar de ejecutar el publisher, escriba el mensaje "salir" tal cual, con todo en minúsculas y sin espacios. 