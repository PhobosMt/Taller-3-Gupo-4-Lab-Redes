// Ver detalles sobre las librerías importadas aquí en el README
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    // Validamos que nos hayan pasado IP, puerto del broker y partido al que se suscribe como argumentos
    if (argc != 4)
    {
        fprintf(stderr, "Uso: %s <ip_broker> <puerto> <partido>\n", argv[0]);
        fprintf(stderr, "Use 'ALL' si quiere recibir noticias sobre todos los partidos\n");
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
    int fd_suscriptor_skt = socket(AF_INET, SOCK_STREAM, 0);

    if (fd_suscriptor_skt < 0)
    {
        perror("socket");
        exit(1);
    }

    printf("Socket creado correctamente, fd = %d\n", fd_suscriptor_skt);

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
    int result_connect = connect(fd_suscriptor_skt, (struct sockaddr *)&addr_broker, sizeof(addr_broker));

    if (result_connect < 0)
    {
        perror("connect");
        exit(1);
    }

    printf("Conectado al broker en %s:%s\n", argv[1], argv[2]);

    // =============== IDENTIFICARSE COMO SUSCRIPTOR DE UN PARTIDO =================

    // snprintf(): arma el mensaje de registro combinando el texto fijo
    // "SUB " con el partido recibido por argumento (argv[3]), respetando
    // el tamaño maximo del buffer para no desbordarlo
    char mensaje_registro[BUF_SIZE];
    snprintf(mensaje_registro, sizeof(mensaje_registro), "SUB %s\n", argv[3]);

    /**
     * send(): envia bytes por un socket ya conectado.
     * 
     * - fd_suscriptor_skt: el socket conectado al broker
     * - mensaje_registro: los datos a enviar ("SUB partido\n")
     * - strlen(mensaje_registro): cuantos bytes enviar
     * - 0: flags, sin opciones especiales
     * 
     * Retorna la cantidad de bytes efectivamente enviados, o -1 si hubo error.
     */
    int bytes_enviados = send(fd_suscriptor_skt, mensaje_registro, strlen(mensaje_registro), 0);

    if (bytes_enviados < 0)
    {
        perror("send");
        exit(1);
    }

    printf("Suscrito al partido '%s'\n", argv[3]);

    // =============== ESPERAR NOTICIAS =================

    printf("Esperando noticias...\n\n");

    char buffer[BUF_SIZE];
    int bytes_leidos;

    /**
     * recv(): recibe bytes desde el socket conectado al broker.
     * 
     * - fd_suscriptor_skt: el socket conectado
     * - buffer: donde se escriben los bytes recibidos
     * - sizeof(buffer) - 1: maximo de bytes a escribir (deja espacio para '\0')
     * - 0: flags, sin opciones especiales
     * 
     * Retorna la cantidad de bytes leidos (> 0), 0 si el broker cerro
     * la conexion, o -1 si hubo un error.
     * 
     * La asignacion dentro de la condicion del while ejecuta recv() en
     * cada vuelta, guarda el resultado en bytes_leidos, y usa ese mismo
     * valor para decidir si el bucle continua (> 0) o termina.
     */
    while ((bytes_leidos = recv(fd_suscriptor_skt, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        // recv() no agrega el terminador nulo automaticamente, hay que
        // ponerlo manualmente antes de tratar "buffer" como string
        buffer[bytes_leidos] = '\0';
        printf("%s", buffer);
        // fflush(): fuerza a que la noticia se muestre de inmediato en
        // pantalla, sin esperar a que el buffer de salida se llene
        fflush(stdout);
    }

    // Distinguimos entre una desconexion ordenada (0) y un error real (negativo)
    if (bytes_leidos == 0)
    {
        printf("\nEl broker cerro la conexion\n");
    }
    else
    {
        perror("recv");
    }

    return 0;
}