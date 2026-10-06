// Ver detalles sobre las librerías importadas aquí en el README
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/select.h>

// Tamaño constante de nuestro buffer de recepción: 1024 bytes
#define BUF_SIZE 1024
// Número de clientes máximo que soporta el broker
#define MAX_CLIENTES 50


// ======================== ESTRUCTURAS ============================

/**
 * Esta enumeración es usada para darle un rol a los clientes
 * Por clientes nos referimos a los suscriptores y publicadores que se
 * comunicarán con el buffer para recibir o publicar noticias deportivas
 * sobre los partidos
 */

typedef enum
{
    ROL_DESCONOCIDO,
    ROL_PUBLICADOR,
    ROL_SUSCRIPTOR
} Rol;

// Esta estructura representa a un cliente

typedef struct
{
    int fd; //file descriptor/Socket que usa el cliente
    Rol rol;
    char partido[64]; // solo tiene sentido si el rol es ROL_SUSCRIPTOR
} Cliente;


// ========================== FUNCIONES AUXILIARES ======================================

// Quita al cliente en la posición "indice" del arreglo, cerrando su socket
void quitar_cliente(Cliente clientes[], int *num_clientes, int indice)
{
    close(clientes[indice].fd);

    // Copiamos TODA la estructura del último cliente, no solo su fd
    // Al sacar a un cliente, damos paso a los demás
    clientes[indice] = clientes[*num_clientes - 1];

    (*num_clientes)--;
}

// Quita el '\n' y/o '\r' del final de un string, si los tiene
void quitar_salto_de_linea(char *s)
{
    size_t len = strlen(s);

    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
    {
        s[--len] = '\0';
    }
}

// Envía la noticia a todos los suscriptores del partido indicado (o "ALL")
void difundir_mensaje(Cliente clientes[], int num_clientes, const char *partido, const char *mensaje)
{
    char linea[BUF_SIZE];
    snprintf(linea, sizeof(linea), "%s: %s\n", partido, mensaje);

    for (int i = 0; i < num_clientes; i++)
    {
        if (clientes[i].rol != ROL_SUSCRIPTOR)
        {
            continue;
        }

        if (strcasecmp(clientes[i].partido, partido) == 0 || strcasecmp(clientes[i].partido, "ALL") == 0)
        {
            send(clientes[i].fd, linea, strlen(linea), 0);
        }
    }
}

int main() 
{
    // =============== CREACIÓN DEL FD DEL SOCKET =================
    /**
     * argumentos de socket:
     * 
     * - AF_INET: la usamos para indicar que usaremos direcciones IPv4
     * - SOCK_STREAM: con esto indicamos el protocolo, TCP en este caso
     * - El 0 indica usar el protocolo por defecto teniendo en cuenta los dos
     *   anteriores argumentos
     */
    int fd_broker_skt = socket(AF_INET, SOCK_STREAM, 0);

    if (fd_broker_skt < 0) 
    {
        perror("socket");
        exit(1);
    }

    printf("Socket creado correctamente, fd = %d\n", fd_broker_skt);

    // =============== CREACIÓN DE LA DIRECCIÓN DEL SOCKET =================

    struct sockaddr_in addr_broker;
    memset(&addr_broker, 0, sizeof(addr_broker));

    addr_broker.sin_family = AF_INET;
    addr_broker.sin_addr.s_addr = INADDR_ANY;
    addr_broker.sin_port = htons(9000); //Usaremos el puerto 9000 para recibir conexiones

    int result_bind = bind(fd_broker_skt, (struct sockaddr *)&addr_broker, sizeof(addr_broker));

    if (result_bind < 0) 
    {
        perror("bind");
        exit(1);
    }

    printf("Bind exitoso en el puerto 9000\n");

    // =============== SOCKET EN ESTADO LISTENING =================

    int result_listen = listen(fd_broker_skt, 10);

    if (result_listen < 0)
    {
        perror("listen");
        exit(1);
    }

    printf("Broker escuchando en el puerto 9000...\n");

    // =============== ARREGLO PARA GUARDAR LOS CLIENTES CONECTADOS =================

    // Cada posición del arreglo guarda el fd de un cliente conectado
    Cliente clientes[MAX_CLIENTES];
    // Cuántos clientes hay conectados actualmente
    int num_clientes = 0;

    
    while(1)
    {
        // =============== ARMAR EL FD_SET PARA select() =================
        fd_set fds_lectura;

        // Vaciamos el conjunto antes de llenarlo
        FD_ZERO(&fds_lectura);

        // Agregamos el socket de escucha, siempre debe estar vigilado
        FD_SET(fd_broker_skt, &fds_lectura);

        // select() necesita saber el fd más alto que le estamos pasando
        int fd_max = fd_broker_skt;

        // Agregamos también todos los clientes ya conectados (por ahora, ninguno)
        for (int i = 0; i < num_clientes; i++)
        {
            FD_SET(clientes[i].fd, &fds_lectura);

            if (clientes[i].fd > fd_max)
            {
                fd_max = clientes[i].fd;
            }
        }

        // =============== LLAMAR A select() =================

        /**
         * argumentos de select:
         * 
         * - fd_max + 1: el rango de fds que el kernel debe revisar
         * - &fds_lectura: el conjunto que queremos vigilar para lectura
         * - NULL, NULL: no nos interesa vigilar escritura ni errores
         * - NULL: sin timeout, se bloquea indefinidamente
         */
        int listos = select(fd_max + 1, &fds_lectura, NULL, NULL, NULL);

        if (listos < 0)
        {
            perror("select");
            exit(1);
        }

        // =============== ¿HAY UNA CONEXIÓN NUEVA? =================

        if (FD_ISSET(fd_broker_skt, &fds_lectura))
        {
            struct sockaddr_in addr_cliente;
            socklen_t len_addr_cliente = sizeof(addr_cliente);

            int fd_nuevo_cliente = accept(fd_broker_skt, (struct sockaddr *)&addr_cliente, &len_addr_cliente);

            if (fd_nuevo_cliente < 0)
            {
                perror("accept");
            }
            else
            {
                clientes[num_clientes].fd = fd_nuevo_cliente;
                clientes[num_clientes].rol = ROL_DESCONOCIDO;
                // Para evitar que haya campos con datos basura que generen bugs en el campo de partido
                memset(clientes[num_clientes].partido, 0, sizeof(clientes[num_clientes].partido));
                num_clientes++;

                printf("Cliente conectado, fd = %d\n", fd_nuevo_cliente);
            }
        }

        // =============== ¿ALGÚN CLIENTE YA CONECTADO MANDÓ DATOS? =================

        char buffer[BUF_SIZE];

        for (int i = 0; i < num_clientes; i++)
        {
            if (!FD_ISSET(clientes[i].fd, &fds_lectura))
            {
                continue; // este cliente no tuvo actividad, pasamos al siguiente
            }

            int bytes_leidos = recv(clientes[i].fd, buffer, sizeof(buffer) - 1, 0);

            if (bytes_leidos < 0)
            {
                perror("recv");
                quitar_cliente(clientes, &num_clientes, i);
                i--;
            }
            else if (bytes_leidos == 0)
            {
                printf("Cliente (fd = %d) cerro la conexion\n", clientes[i].fd);
                quitar_cliente(clientes, &num_clientes, i);
                i--;
            }
            else
            {
                buffer[bytes_leidos] = '\0';
                quitar_salto_de_linea(buffer);

                // Se debe reconocer primero que rol se está comunicando
                if (clientes[i].rol == ROL_DESCONOCIDO)
                {
                    // Para los suscriptores es SUB tópico
                    if (strncmp(buffer, "SUB ", 4) == 0)
                    {
                        clientes[i].rol = ROL_SUSCRIPTOR;
                        strncpy(clientes[i].partido, buffer + 4, sizeof(clientes[i].partido) - 1);
                        clientes[i].partido[sizeof(clientes[i].partido) - 1] = '\0';

                        printf("Nuevo suscriptor (fd = %d) al partido '%s'\n", clientes[i].fd, clientes[i].partido);
                    }
                    // Los publicadores deben empezar anunciándose con PUB (nada más)
                    else if (strcmp(buffer, "PUB") == 0)
                    {
                        clientes[i].rol = ROL_PUBLICADOR;

                        printf("Nuevo publicador conectado (fd = %d)\n", clientes[i].fd);
                    }
                    // Para manejar mensajes que no se reconozcan como de publicador/suscriptor
                    else
                    {
                        printf("Mensaje de registro desconocido (fd = %d): %s\n", clientes[i].fd, buffer);
                    }
                }

                // Caso para publicadores ya identificados
                else if (clientes[i].rol == ROL_PUBLICADOR)
                {
                    char *separador = strchr(buffer, ':');

                    if (separador == NULL)
                    {
                        printf("Formato invalido de publicador (fd = %d): %s\n", clientes[i].fd, buffer);
                    }
                    else
                    {
                        *separador = '\0';
                        char *partido = buffer;
                        char *mensaje = separador + 1;

                        while (*mensaje == ' ')
                        {
                            mensaje++;
                        }

                        printf("Publicando '%s' -> partido '%s' (enviado por publicador fd=%d)\n", mensaje, partido, clientes[i].fd);
                        difundir_mensaje(clientes, num_clientes, partido, mensaje);
                    }
                }
            }
        }
    }

    // =============== CERRAR LOS SOCKETS =================

    // Cerramos el socket de cada cliente que se haya conectado
    for (int i = 0; i < num_clientes; i++)
    {
        close(clientes[i].fd);
    }

    // Y luego el socket de escucha del broker
    close(fd_broker_skt);

    printf("Conexion cerrada, broker finalizado\n");

    return 0;
}