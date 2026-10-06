// Ver detalles sobre las librerías importadas aquí en el README
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    // Validamos que nos hayan pasado IP y puerto del broker como argumentos
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <ip_broker> <puerto>\n", argv[0]);
        exit(1);
    }

    // =============== CREACIÓN DEL SOCKET =================

    /**
     * socket(): le pide al sistema operativo un socket nuevo.
     * 
     * - AF_INET: familia de direcciones IPv4
     * - SOCK_STREAM: tipo de socket orientado a flujo y con conexion (TCP)
     * - 0: protocolo por defecto para esta combinacion (TCP)
     * 
     * Retorna un file descriptor (entero) que identifica este socket,
     * o -1 si hubo un error.
     */
    int fd_publicador_skt = socket(AF_INET, SOCK_STREAM, 0);

    if (fd_publicador_skt < 0)
    {
        perror("socket");
        exit(1);
    }

    printf("Socket creado correctamente, fd = %d\n", fd_publicador_skt);

    // =============== ARMAR LA DIRECCIÓN DEL BROKER =================

    struct sockaddr_in addr_broker;
    memset(&addr_broker, 0, sizeof(addr_broker));

    addr_broker.sin_family = AF_INET;
    // htons(): convierte el puerto (entero de 16 bits) del formato que usa
    // la CPU al formato estandar de red (big-endian). atoi() convierte
    // el puerto recibido como texto (argv[2]) a un numero entero.
    addr_broker.sin_port = htons(atoi(argv[2]));

    /**
     * inet_pton(): traduce una IP en texto legible (ej. "192.168.1.50")
     * a su representacion binaria de red, y la escribe directamente en
     * addr_broker.sin_addr. Retorna 1 si tuvo exito, 0 si el texto no
     * era una IP valida, o -1 si hubo un error mayor.
     */
    if (inet_pton(AF_INET, argv[1], &addr_broker.sin_addr) <= 0)
    {
        fprintf(stderr, "Direccion IP invalida: %s\n", argv[1]);
        exit(1);
    }

    // =============== CONECTARSE AL BROKER =================

    /**
     * connect(): inicia activamente una conexion hacia la direccion
     * indicada (IP + puerto del broker). A diferencia de bind()/listen(),
     * que son para el lado servidor (pasivo), connect() es la llamada
     * que usa el lado cliente para "marcar" hacia un servidor que ya
     * esta escuchando. Aqui es donde ocurre el three-way handshake de TCP.
     * 
     * El cast (struct sockaddr *) es necesario porque connect() espera
     * un puntero al tipo generico sockaddr, y nosotros construimos la
     * version especifica sockaddr_in (misma base de memoria, por eso
     * el cast es valido).
     */
    int result_connect = connect(fd_publicador_skt, (struct sockaddr *)&addr_broker, sizeof(addr_broker));

    if (result_connect < 0)
    {
        perror("connect");
        exit(1);
    }

    printf("Conectado al broker en %s:%s\n", argv[1], argv[2]);

    // =============== IDENTIFICARSE COMO PUBLICADOR =================

    const char *mensaje_registro = "PUB\n";

    /**
     * send(): envia bytes por un socket ya conectado.
     * 
     * - fd_publicador_skt: el socket conectado al broker
     * - mensaje_registro: los datos a enviar
     * - strlen(mensaje_registro): cuantos bytes enviar
     * - 0: flags, sin opciones especiales
     * 
     * Retorna la cantidad de bytes efectivamente enviados, o -1 si hubo error.
     */
    int bytes_enviados = send(fd_publicador_skt, mensaje_registro, strlen(mensaje_registro), 0);

    if (bytes_enviados < 0)
    {
        perror("send");
        exit(1);
    }

    printf("Registrado como publicador ante el broker\n");

    // =============== BUCLE DE ENVÍO DE NOTICIAS =================

    printf("Formato de entrada: PARTIDO:mensaje   (ej. Arsenal-Liverpool:Gol al minuto 30)\n");
    printf("Escriba 'salir' para terminar.\n\n");

    char linea[BUF_SIZE];

    while (1)
    {
        printf("> ");
        // fflush(): fuerza a que el prompt "> " se muestre de inmediato,
        // sin esperar a un salto de linea o a que el buffer de salida se llene
        fflush(stdout);

        // fgets(): lee una linea completa de teclado (incluyendo el '\n'),
        // hasta sizeof(linea) - 1 caracteres. Retorna NULL si hay error
        // o se alcanza el fin de la entrada (ej. Ctrl+D).
        if (!fgets(linea, sizeof(linea), stdin))
        {
            break;
        }

        // strncmp(): compara solo los primeros 5 caracteres de "linea"
        // contra "salir", para permitir el \n que fgets() incluye al final
        if (strncmp(linea, "salir", 5) == 0)
        {
            break;
        }

        // strchr(): busca el caracter ':' dentro de "linea"; valida que
        // el formato ingresado sea PARTIDO:mensaje antes de enviarlo
        if (strchr(linea, ':') == NULL)
        {
            printf("Formato invalido, use PARTIDO:mensaje\n");
            continue;
        }

        // send(): reenvia la linea completa (partido:mensaje\n) al broker
        int bytes = send(fd_publicador_skt, linea, strlen(linea), 0);

        if (bytes < 0)
        {
            perror("send");
            break;
        }
    }

    return 0;
}