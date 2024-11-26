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

int main(int argc, char *argv[])
{
    int sd, err;
    int optval = 1;
    int flag = 1;
    char *service;
    struct addrinfo hints, *res;

    if (argc != 2) {
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
    if (err != 0) {
        fprintf(stderr, "Error: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);    
    if (sd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    err = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (err < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    err = bind(sd, res->ai_addr, res->ai_addrlen);
    if (err < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    err = listen(sd, SOMAXCONN);
    if (err < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (flag == 1) {
        rxb_t rxb;
        int ns;
        int client_active = 1;  // Flag per gestire la connessione client

        ns = accept(sd, NULL, NULL);
        if (ns < 0) {
            if (errno == EINTR)
                continue;
            perror("accept");
            exit(EXIT_FAILURE);
        }

        rxb_init(&rxb, 64 * 1024);

        // Ciclo interno per gestire multiple richieste dallo stesso client
        while (client_active) {
            char str1[1024];
            size_t str1_len;
            
            memset(str1, 0, sizeof(str1));
            str1_len = sizeof(str1)-1;

            err = rxb_readline(&rxb, ns, str1, &str1_len);
            if (err < 0) {
                if (errno == ECONNRESET || errno == EPIPE) {
                    printf("Client disconnesso\n");
                    break;
                }
                perror("rxb_readline");
                break;
            }

            if (str1_len == 0) {  // Connection closed by client
                printf("Client ha chiuso la connessione\n");
                break;
            }

            printf("Stringa ricevuta: %s\n", str1);

            if (strcmp(str1, "fine\n") == 0 || strcmp(str1, "fine") == 0) {
                flag = 0;
                client_active = 0;
                printf("Comunicazione terminata su richiesta del client\n");
            } else {
                int pip1[2]; // pipe per grep -> sort
                int pip2[2]; // pipe per sort -> socket

                // Creiamo entrambe le pipe
                if (pipe(pip1) < 0 || pipe(pip2) < 0) {
                    perror("pipe");
                    break;
                }

                // Primo fork per grep
                int pid1 = fork();
                if (pid1 < 0) {
                    perror("fork");
                    break;
                } 
                else if (pid1 == 0) {  // Primo figlio (grep)
                    close(pip1[0]);
                    close(pip2[0]);
                    close(pip2[1]);
                    
                    dup2(pip1[1], STDOUT_FILENO);
                    close(pip1[1]);
                    
                    execlp("grep", "grep", str1, "conto_corrente.txt", NULL);
                    perror("execlp grep");
                    exit(EXIT_FAILURE);
                }

                // Secondo fork per sort
                int pid2 = fork();
                if (pid2 < 0) {
                    perror("fork");
                    break;
                }
                else if (pid2 == 0) {  // Secondo figlio (sort)
                    close(pip1[1]);
                    close(pip2[0]);
                    
                    dup2(pip1[0], STDIN_FILENO);
                    close(pip1[0]);
                    
                    dup2(pip2[1], STDOUT_FILENO);
                    close(pip2[1]);
                    
                    execlp("sort", "sort", "-r", NULL);
                    perror("execlp sort");
                    exit(EXIT_FAILURE);
                }

                // Processo padre
                close(pip1[0]);
                close(pip1[1]);
                close(pip2[1]);

                // Leggiamo dalla seconda pipe e scriviamo sul socket
                char buffer[4096];
                ssize_t bytes_read;
                
                while ((bytes_read = read(pip2[0], buffer, sizeof(buffer))) > 0) {
                    ssize_t bytes_written = write(ns, buffer, bytes_read);
                    if (bytes_written < 0) {
                        if (errno == EPIPE) {
                            printf("Client disconnesso durante la scrittura\n");
                            client_active = 0;
                            break;
                        }
                        perror("write to socket");
                        break;
                    }
                }
                close(pip2[0]);
                
                // Attendiamo la terminazione dei processi figli
                waitpid(pid1, NULL, 0);
                waitpid(pid2, NULL, 0);
            }
        }
        rxb_destroy(&rxb);
        close(ns);
    }
    close(sd);
    return 0;
}