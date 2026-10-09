// Client side implementation of UDP client-server model

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>

#define MAXLINE 1024
#define MAX_CLIENTES 50

typedef struct {
    struct sockaddr_in addr;
    char partido[64];
} SuscriptorUDP;

void quitar_salto_de_linea(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
        s[--len] = '\0';
    }
}

int main(int argc, char *argv[]) {
    int puerto = 9000;
    if (argc ==2) {
        puerto = atoi(argv[1]);
    }

    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    SuscriptorUDP suscriptores[MAX_CLIENTES];
    int num_suscriptores = 0;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(puerto);

    if (bind(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Broker UDP escuchando en el puerto %d...\n\n", puerto);

    char buffer[MAXLINE];
    socklen_t len = sizeof(servaddr);
    
    while (1) {
        int bytes_leidos = recvfrom(sockfd, buffer, sizeof(buffer)-1, 0,
         (struct sockaddr*)&cliaddr, &len); 

        if (bytes_leidos < 0) {
            perror("recvfrom");
            continue;
        }
        
        buffer[bytes_leidos] = '\0';
        quitar_salto_de_linea(buffer);

        if (strncmp(buffer, "SUB ", 4) == 0) {
            if (num_suscriptores < MAX_CLIENTES) {
                suscriptores[num_suscriptores].addr = cliaddr;
                strncpy(suscriptores[num_suscriptores].partido, buffer+4, sizeof(suscriptores[num_suscriptores].partido) -1);
                suscriptores[num_suscriptores].partido[sizeof(suscriptores[num_suscriptores].partido)-1] = '\0';

                printf("Nuevo suscriptor UDP al partido '%s'\n", suscriptores[num_suscriptores].partido);
                num_suscriptores++;
            }
        }
        else if (strcmp(buffer, "PUB") == 0) {
            printf("Nuevo publicador UDP detectado\n");
        }
        else {
            char *separador = strchr(buffer, ':');
            if (separador != NULL) {
                *separador = '\0';
                char *partido = buffer;
                char *mensaje = separador +1;

                while (*mensaje==' ') mensaje++;
                printf("Publicando '%s' -> partido '%s'\n", mensaje, partido);

                char linea_difusion[MAXLINE];
                snprintf(linea_difusion, sizeof(linea_difusion), "%s: %s\n", partido, mensaje);

                for(int i=0; i < num_suscriptores; i++) {
                    if (strcasecmp(suscriptores[i].partido, partido) ==0 || strcasecmp(suscriptores[i].partido, "ALL") ==0) {
                        sendto(sockfd, linea_difusion, strlen(linea_difusion), 0, (struct sockaddr *) &suscriptores[i].addr, sizeof(suscriptores[i].addr));    
                    }
                }
            }
        }
    }
    close(sockfd);
    return 0;
}