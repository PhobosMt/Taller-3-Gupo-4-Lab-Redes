# Taller-3-Gupo-4-Lab-Redes
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

## **publicador_tcp.c**

Librerías utilizadas:

- `#include <stdio.h>` **(estándar de C)**: La usamos para `printf()` (mensajes informativos y el prompt de entrada), `fprintf()` (mensajes de error dirigidos a `stderr`), `perror()` (errores de syscalls), `fgets()` (leer líneas desde el teclado) y `snprintf()` (armar el mensaje de suscripción/publicación con formato).

- `#include <stdlib.h>` **(estándar de C)**: La usamos para `exit()` (terminar el programa ante una falla) y `atoi()` ("ASCII to integer", convierte el puerto recibido como texto por línea de comandos a un número entero).

- `#include <string.h>` **(estándar de C)**: La usamos para `memset()` (inicializar la estructura de dirección), `strlen()` (medir los mensajes antes de enviarlos), `strncmp()` (reconocer el comando `"salir"`) y `strchr()` (validar que el mensaje tenga el formato `equipo:mensaje`).

- `#include <sys/socket.h>` **(POSIX, no estándar de C)**: La usamos para `socket()` (crear el socket) y `send()` (enviar los mensajes de registro y las noticias al broker).

- `#include <arpa/inet.h>` **(POSIX, no estándar de C)**: La usamos para la estructura `struct sockaddr_in`, la función `inet_pton()` (convertir la IP de texto a formato binario de red) y `htons()` (convertir el puerto a formato de red).


## **suscriptor_tcp.c**

Librerías utilizadas:

- `#include <stdio.h>` **(estándar de C)**: La usamos para `printf()` (mensajes informativos y las noticias recibidas), `perror()` (errores de syscalls) y `snprintf()` (armar el mensaje de suscripción `SUB <equipo>`).

- `#include <stdlib.h>` **(estándar de C)**: La usamos para `exit()` (terminar el programa ante una falla) y `atoi()` (convertir el puerto recibido por línea de comandos a número entero).

- `#include <string.h>` **(estándar de C)**: La usamos para `memset()` (inicializar la estructura de dirección) y `strlen()` (medir el mensaje de registro antes de enviarlo).

- `#include <sys/socket.h>` **(POSIX, no estándar de C)**: La usamos para `socket()` (crear el socket), `send()` (enviar el mensaje de suscripción) y `recv()` (recibir las noticias que el broker reenvía).

- `#include <arpa/inet.h>` **(POSIX, no estándar de C)**: La usamos para `struct sockaddr_in`, `inet_pton()` y `htons()`, igual que en el publicador.