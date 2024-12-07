#include <stdint.h>
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "rxb.h"
#include "utils.h"
#include <sys/wait.h>
#include <ctype.h>

double sum_numbers(char* buf) {
    double sum = 0;
    char* p = buf;
    while (*p) {
        // Se troviamo un numero lo convertiamo
        if (*p >= '0' && *p <= '9') {
            sum += atoi(p);
            printf("%c\n",*p);
            printf("%lf\n",sum);
            // Saltiamo fino alla virgola
            while (*p && *p != ',') p++;
        }
        p++;
    }
    return sum;
}


int main(int argc, char *argv[])
{
    int sd, err;
    int optval = 1;
    int flag = 1;
    char *service;
    struct addrinfo hints, *res;

    if (argc != 2)
    {
        fprintf(stderr, "Usage:\n\t%s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);
    service = argv[1];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    err = getaddrinfo(NULL, service, &hints, &res);
    if (err != 0)
    {
        fprintf(stderr, "Error: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    err = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    err = bind(sd, res->ai_addr, res->ai_addrlen);
    if (err < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    err = listen(sd, SOMAXCONN);
    if (err < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    rxb_t rxb;
    int ns;
    char str1[1024];
    size_t str1_len;

    char str2[1024];
    size_t str2_len;

    while (1)
    {
        ns = accept(sd, NULL, NULL);
        if (ns < 0)
        {
            if (errno == EINTR)
                continue;
            perror("accept");
            exit(EXIT_FAILURE);
        }
        int pid00 = fork();
        if (pid00 == 0)
            while (1)
            {
                /** REGIONE */
                rxb_init(&rxb, 64 * 1024);
                memset(str1, 0, sizeof(str1));
                str1_len = sizeof(str1) - 1;

                err = rxb_readline(&rxb, ns, str1, &str1_len);
                if (err < 0)
                {
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }

                printf("REGIONE ricevuto: %s\n", str1);

                /**ANNATA */
                rxb_init(&rxb, 64 * 1024);
                memset(str2, 0, sizeof(str1));
                str2_len = sizeof(str1) - 1;

                err = rxb_readline(&rxb, ns, str2, &str2_len);
                if (err < 0)
                {
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }
                printf("NUMERO ricevuta: %s\n", str2);

                if (strcmp(str1, "fine") != 0 && strcmp(str2, "fine") != 0)
                {
                    int pip1[2]; // pipe per grep -> sort
                    int pip2[2]; // pipe per sort -> socket

                    // Creiamo entrambe le pipe
                    if (pipe(pip1) < 0 || pipe(pip2) < 0)
                    {
                        perror("pipe");
                        exit(EXIT_FAILURE);
                    }

                    // Primo fork per grep
                    int pid1 = fork();
                    if (pid1 < 0)
                    {
                        perror("fork");
                        exit(EXIT_FAILURE);
                    }
                    else if (pid1 == 0)
                    { // Primo figlio (grep)
                        // Chiudiamo i descrittori non necessari
                        close(pip1[0]);
                        close(pip2[0]);
                        close(pip2[1]);

                        // Redirigiamo stdout sulla prima pipe
                        dup2(pip1[1], STDOUT_FILENO);
                        close(pip1[1]);

                        // Eseguiamo sort
                        char pollo[5000];
                        sprintf(pollo, "%s.txt", str1);
                        execlp("sort", "sort", pollo, NULL);
                        perror("execlp sort");
                        exit(EXIT_FAILURE);
                    }

                    // Secondo fork per sort
                    int pid2 = fork();
                    if (pid2 < 0)
                    {
                        perror("fork");
                        exit(EXIT_FAILURE);
                    }
                    else if (pid2 == 0)
                    { // Secondo figlio (sort)
                        // Chiudiamo i descrittori non necessari
                        close(pip1[1]);
                        close(pip2[0]);

                        // Redirigiamo stdin dalla prima pipe
                        dup2(pip1[0], STDIN_FILENO);
                        close(pip1[0]);

                        // Redirigiamo stdout sulla seconda pipe
                        dup2(pip2[1], STDOUT_FILENO);
                        close(pip2[1]);

                        // Eseguiamo head
                        execlp("head", "head","-n", str2, NULL);
                        perror("execlp head");
                        exit(EXIT_FAILURE);
                    }

                    // Processo padre
                    // Chiudiamo i descrittori delle pipe non necessari
                    close(pip1[0]);
                    close(pip1[1]);
                    close(pip2[1]);

                    // Leggiamo dalla seconda pipe e scriviamo sul socket
                    char buffer[4096];
                    ssize_t bytes_read;
                    int numero = 0;
                    int el = atoi(str2);
                    char bufferSupremoDellaMedia[4096];
                    
                    while ((bytes_read = read(pip2[0], buffer, sizeof(buffer))) > 0)
                    {

                        ssize_t bytes_written = write(ns, buffer, bytes_read);
                        if (bytes_written < 0)
                        {
                            perror("write to socket");
                            break;
                        }   

                        write(ns, "\n", 1);
                        numero = sum_numbers(buffer);
                        sprintf(bufferSupremoDellaMedia, " %d\n", (numero/el));
                        bytes_written = write(ns, bufferSupremoDellaMedia, strlen(bufferSupremoDellaMedia));
                        if (bytes_written < 0)
                        {
                            perror("write to socket");
                            break;
                        }   
                        write(ns, "\n", 1);
                    }

                    close(pip2[0]);
                    // Attendiamo la terminazione dei processi figli
                    waitpid(pid1, NULL, 0);
                    waitpid(pid2, NULL, 0);
                }
                else
                {
                    flag = 0;
                    printf("Comunicazione terminata su richiesta del client\n");
                }
            }
        close(ns);
        rxb_destroy(&rxb);
    }
    close(ns);
    close(sd);
    return 0;
}



