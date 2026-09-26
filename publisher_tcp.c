// Para visualizar mensajes y errores
#include <stdio.h>
// Para exit() y atoi()
#include <stdlib.h>
// Para memset()
#include <string.h>
// Función para declarar sockets
#include <sys/socket.h>
// Funciones de conversión de direcciones (inet_pton, htons)
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
    addr_broker.sin_port = htons(atoi(argv[2]));

    //inet_pton: traducción de ips a lenguaje de máquinas y versión legible
    if (inet_pton(AF_INET, argv[1], &addr_broker.sin_addr) <= 0)
    {
        fprintf(stderr, "Direccion IP invalida: %s\n", argv[1]);
        exit(1);
    }

    // =============== CONECTARSE AL BROKER =================

    int result_connect = connect(fd_publicador_skt, (struct sockaddr *)&addr_broker, sizeof(addr_broker));

    if (result_connect < 0)
    {
        perror("connect");
        exit(1);
    }

    printf("Conectado al broker en %s:%s\n", argv[1], argv[2]);

    // =============== IDENTIFICARSE COMO PUBLICADOR =================

    const char *mensaje_registro = "PUB\n";

    int bytes_enviados = send(fd_publicador_skt, mensaje_registro, strlen(mensaje_registro), 0);

    if (bytes_enviados < 0)
    {
        perror("send");
        exit(1);
    }

    printf("Registrado como publicador ante el broker\n");

    // =============== BUCLE DE ENVÍO DE NOTICIAS =================

    printf("Formato de entrada: EQUIPO:mensaje   (ej. Millonarios:Gol al minuto 30)\n");
    printf("Escriba 'salir' para terminar.\n\n");

    char linea[BUF_SIZE];

    while (1)
    {
        printf("> ");
        fflush(stdout);

        if (!fgets(linea, sizeof(linea), stdin))
        {
            break;
        }

        if (strncmp(linea, "salir", 5) == 0)
        {
            break;
        }

        if (strchr(linea, ':') == NULL)
        {
            printf("Formato invalido, use EQUIPO:mensaje\n");
            continue;
        }

        int bytes = send(fd_publicador_skt, linea, strlen(linea), 0);

        if (bytes < 0)
        {
            perror("send");
            break;
        }
    }

    return 0;
}