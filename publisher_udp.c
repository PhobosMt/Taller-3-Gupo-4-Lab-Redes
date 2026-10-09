// Client side implementation of UDP client-server model

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXLINE  1024

int main(int argc, char *argv[]) {

    if (argc !=3) {
        fprintf(stderr, "Uso: %s <ip_broker> <puerto>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in servaddr;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Fill server address info
    servaddr.sin_family = AF_INET;              // IPv4
    servaddr.sin_port   = htons(atoi(argv[2]));          // Server port

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "Direccion IP invalida: %s\n", argv[1]);
        exit(1);
    }



    socklen_t len = sizeof(servaddr);

    // Send message to server
    const char *mensaje_registro = "PUB\n";
    sendto(sockfd, mensaje_registro, strlen(mensaje_registro), 0,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Registrado como publicador UDP ante el broker en %s:%s\n", argv[1], argv[2]);

    // Receive reply from server
    printf("Formato de entrada: PARTIDO:mensaje (ej. Arsenal-Liverpool:Gol al minuto 30)\n");
    printf("Escriba 'salir' para terminar.\n\n");

    char linea[MAXLINE];

    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(linea, sizeof(linea), stdin)) {
            break;
        }
        if (strncmp(linea, "salir", 5) == 0) {
            break;
        }
        if (strchr(linea, ':') == NULL) {
            printf("Formato invalido, use PARTIDO:mensaje\n");
            continue;
        }
        sendto(sockfd, linea, strlen(linea), 0,
               (const struct sockaddr *)&servaddr, sizeof(servaddr));
    }

    close(sockfd);
    return 0;
}