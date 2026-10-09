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

    if (argc !=4) {
        fprintf(stderr, "Uso: %s <ip_broker> <puerto> <partido|ALL>\n", argv[0]);
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
    char mensaje_sub[MAXLINE];
    snprintf(mensaje_sub, sizeof(mensaje_sub), "SUB %s\n", argv[3]);
    sendto(sockfd, mensaje_sub, strlen(mensaje_sub), 0,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Suscrito al partido '%s' ante el broker %s:%s\n", argv[3], argv[1], argv[2]);
    printf("Esperando noticias...\n\n");

    
    char buffer[MAXLINE];
    
    while (1) {
        int bytes_leidos = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
         (struct sockaddr *)&servaddr, &len); 

        if (bytes_leidos < 0) {
            perror("recvfrom");
            break;
        }
        
        buffer[bytes_leidos] = '\0';
        printf("%s", buffer);
        fflush(stdout);
    }

    close(sockfd);
    return 0;
}