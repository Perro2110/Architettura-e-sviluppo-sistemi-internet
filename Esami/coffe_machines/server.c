// inizio 9.03 9.57

// gcc -g -o ./eseC client.c rxb.c rxb.h utils.c utils.h

#include <stdint.h>
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include "utils.h"
#include "rxb.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <ctype.h>

int autorizza(char *str1, char *str2)
{
    return 1;
}

//    ./eseS    numPort
//    argv[0] argv[1]
int main(int argc, char const *argv[])
{
    // inizializzo variabili socket
    int sd, err;
    int optval = 1;
    int flag = 1;
    char *service;
    struct addrinfo hints, *res;
    // Controllo argomenti
    if (argc != 2)
    {
        fputs("errore argomenti, uso: ' eseS [Porta] ' ", stderr);
        exit(EXIT_FAILURE);
    }
    // Ignora SIGPIPE per evitare terminazione su write su socket chiuso
    signal(SIGPIPE, SIG_IGN);
    service = argv[1];

    // configurazione socket
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // IPV4 O IPV6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // PER BIND SU TUTTE LE INTERFACCE

    // risoluzione indirizzo
    err = getaddrinfo(NULL, service, &hints, &res);
    if (err != 0)
    {
        fprintf(stderr, "Error %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }
    // creazione socket
    sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    // Riutilizzo immediato
    err = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("sockopt");
        exit(EXIT_FAILURE);
    }
    // bind
    err = bind(sd, res->ai_addr, res->ai_addrlen);
    if (err < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    // mette socket in ascolto
    err = listen(sd, SOMAXCONN);
    if (err < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    // buffer per la lettura
    rxb_t rxb;
    int ns;
    char str1[1024]; // USERNAME
    size_t str1_len;
    char str2[1024]; // PASSWORD
    size_t str2_len;
    char str3[1024]; // MACCHINARIO
    size_t str3_len;

    // loop principalr del server
    while (1)
    {
        // accetta nuova connessione
        ns = accept(sd, NULL, NULL);
        if (ns < 0)
        {
            if (errno == EINTR)
                continue;
            perror("accept");
            exit(EXIT_FAILURE);
        }
        // fork per gestire il client
        int pid00 = fork();
        if (pid00 < 0)
        {
            perror("fork 00");
            exit(EXIT_FAILURE);
        }
        if (pid00 == 0)
        { // PROCESSO FIGLIO PER GESTIRE CLIENT
            rxb_init(&rxb, 64 * 1024);
            while (1)
            {
                // lettura nome utente
                memset(str1, 0, sizeof(str1));
                str1_len = sizeof(str1) - 1;
                err = rxb_readline(&rxb, ns, str1, &str1_len);
                if (err < 0)
                {
                    perror("rxb_readline username");
                    exit(EXIT_FAILURE);
                }

                // lettura password
                memset(str2, 0, sizeof(str2));
                str2_len = sizeof(str2) - 1;
                err = rxb_readline(&rxb, ns, str2, &str2_len);
                if (err < 0)
                {
                    perror("rxb_readline password");
                    exit(EXIT_FAILURE);
                }

                // lettura macchinario
                memset(str3, 0, sizeof(str3));
                str3_len = sizeof(str3) - 1;
                err = rxb_readline(&rxb, ns, str3, &str3_len);
                if (err < 0)
                {
                    perror("rxb_readline macchinario");
                    exit(EXIT_FAILURE);
                }

                // se autorizzato
                if (strcmp(str1, "fine") != 0 && strcmp(str2, "fine") != 0 && strcmp(str3, "fine") != 0 && autorizza(str1, str2))
                {
                    int pip1[2]; // pipe per sort -> head
                    int pip2[2]; // pipe head -> socket //preferisco non scrivere/redirezionare direttamente su ns
                    // creazione delle pipe
                    if (pipe(pip1) < 0 || pipe(pip2) < 0)
                    {
                        perror("pipe");
                        exit(EXIT_FAILURE);
                    }
                    // fork per sort
                    int pid1 = fork();
                    if (pid1 < 0)
                    {
                        perror("fork 1");
                        exit(EXIT_FAILURE);
                    }
                    if (pid1 == 0)
                    { // PRIMO FIGLIO (SORT)
                        close(pip1[0]);
                        close(pip2[0]);
                        close(pip2[1]);

                        dup2(pip1[1], STDOUT_FILENO);
                        close(pip1[1]);

                        // eseguo sort
                        char txt[5000];
                        /*           ..../%s.txt        */
                        sprintf(txt, "%s.txt", str3);
                        execlp("sort", "sort", txt, NULL);
                        perror("execlp sort");
                        exit(EXIT_FAILURE);
                    }
                    // FORK PER HEAD
                    int pid2 = fork();
                    if (pid2 < 0)
                    {
                        perror("fork 2");
                        exit(EXIT_FAILURE);
                    }
                    if (pid2 == 0)
                    { // FIGLIO (HEAD)
                        close(pip1[1]);
                        close(pip2[0]);

                        dup2(pip1[0], STDIN_FILENO);
                        close(pip1[0]);

                        dup2(pip2[1], STDOUT_FILENO);
                        close(pip2[1]);

                        // eseguo head
                        execlp("head", "head", "-n", "10", NULL);
                        perror("execlp head");
                        exit(EXIT_FAILURE);
                    }
                    // processo padre
                    close(pip1[0]);
                    close(pip1[1]);
                    close(pip2[1]);

                    waitpid(pid1, NULL, 0);
                    waitpid(pid2, NULL, 0);

                    char buffer[4096];
                    ssize_t bytes_read;

                    while ((bytes_read = read(pip2[0], buffer, sizeof(buffer))) > 0)
                    {
                        ssize_t bytes_written = write_all(ns, buffer, bytes_read);
                        if (bytes_written < 0)
                        {
                            perror("write to socket");
                            break;
                        }
                        write(ns, "\n", 1);
                        write_all(ns, "==fin==", sizeof("==fin=="));
                        write(ns, "\n", 1);
                        close(pip2[0]);
                    }
                }
                else
                {
                    printf("comunicazione terminata da utente o autorizzazione negata \n");
                    rxb_destroy(&rxb);
                    exit(0);
                }
            }
        }
        close(ns);
    }
    close(ns);
    close(sd);
    return -1;
}
