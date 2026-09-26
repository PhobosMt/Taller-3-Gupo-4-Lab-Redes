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
    // Validamos que nos hayan pasado IP, puerto del broker y equipo al que se suscribe como argumentos
    if (argc != 4)
    {
        fprintf(stderr, "Uso: %s <ip_broker> <puerto> <equipo>\n", argv[0]);
        fprintf(stderr, "Use 'ALL' si quiere recibir noticias sobre todos los equipos\n");
        exit(1);
    }

    // =============== CREACIÓN DEL SOCKET =================

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
    addr_broker.sin_port = htons(atoi(argv[2]));

    //inet_pton: traducción de ips a lenguaje de máquinas y versión legible
    if (inet_pton(AF_INET, argv[1], &addr_broker.sin_addr) <= 0)
    {
        fprintf(stderr, "Direccion IP invalida: %s\n", argv[1]);
        exit(1);
    }

    // =============== CONECTARSE AL BROKER =================

    int result_connect = connect(fd_suscriptor_skt, (struct sockaddr *)&addr_broker, sizeof(addr_broker));

    if (result_connect < 0)
    {
        perror("connect");
        exit(1);
    }

    printf("Conectado al broker en %s:%s\n", argv[1], argv[2]);

    // =============== IDENTIFICARSE COMO SUSCRIPTOR DE UN EQUIPO =================

    char mensaje_registro[BUF_SIZE];
    snprintf(mensaje_registro, sizeof(mensaje_registro), "SUB %s\n", argv[3]);

    int bytes_enviados = send(fd_suscriptor_skt, mensaje_registro, strlen(mensaje_registro), 0);

    if (bytes_enviados < 0)
    {
        perror("send");
        exit(1);
    }

    printf("Suscrito al equipo '%s'\n", argv[3]);

    // =============== ESPERAR NOTICIAS =================

    printf("Esperando noticias...\n\n");

    char buffer[BUF_SIZE];
    int bytes_leidos;

    while ((bytes_leidos = recv(fd_suscriptor_skt, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_leidos] = '\0';
        printf("%s", buffer);
        fflush(stdout);
    }

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