// gcc -g -o ./eseC client.c rxb.c rxb.h utils.c utils.h
// 9.55 10.36
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "utils.h"
#include "rxb.h"
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

int autorizza( char *username,  char *password)
{
    return 1;
}

int main(int argc, char const *argv[])
{
    int sd, err;
    int optval = 1;
    int flag = 1;
    char *service;

    struct addrinfo hints, *res;

    if (argc != 2)
    {
        fputs("Errore numero argomenti\n", stderr);
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
        fprintf(stderr, "Errore %s\n", gai_strerror(err));
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
    char str3[1024];
    size_t str3_len;

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
        if (pid00 < 0)
        {
            perror("fork 00");
            exit(EXIT_FAILURE);
        }
        if (pid00 == 0)
        { // FIGLIO CAPO CONNESSIONE
            rxb_init(&rxb, 64 * 1024);
            while (1)
            {
                // lettura username
                memset(str1, 0, sizeof(str1));
                str1_len = sizeof(str1) - 1;
                err = rxb_readline(&rxb, ns, str1, &str1_len);
                if (err < 0)
                {
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }

                // lettura password
                memset(str2, 0, sizeof(str2));
                str2_len = sizeof(str2) - 1;
                err = rxb_readline(&rxb, ns, str2, &str2_len);
                if (err < 0)
                {
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }

                // lettura categoria
                memset(str3, 0, sizeof(str3));
                str3_len = sizeof(str3) - 1;
                err = rxb_readline(&rxb, ns, str3, &str3_len);
                if (err < 0)
                {
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }

                if (strcmp(str1, "fine") != 0 && strcmp(str2, "fine") != 0 && strcmp(str3, "fine") != 0 && autorizza(str1, str2) == 1)
                {
                    int pip1[2]; // sort -> head
                    int pip2[2]; // head -> cut
                    int pip3[2]; // cut -> capo connessione

                    if (pipe(pip1) < 0 || pipe(pip2) < 0 || pipe(pip3) < 0)
                    {
                        perror("pipe");
                        exit(EXIT_FAILURE);
                    }

                    int pid1 = fork();
                    if (pid1 < 0)
                    {
                        perror("fork 1");
                        exit(EXIT_FAILURE);
                    }
                    if (pid1 == 0)
                    { // primo figlio sort
                        close(pip1[0]);
                        close(pip2[0]);
                        close(pip2[1]);
                        close(pip3[0]);
                        close(pip3[1]);

                        dup2(pip1[1], STDOUT_FILENO);
                        close(pip1[1]);

                        char buffy[5000];
                        sprintf(buffy, "%s.txt", str3);
                        execlp("sort", "sort", "-n", "-r", buffy, NULL);
                        perror("sort");
                        exit(EXIT_FAILURE);
                    }
                    int pid2 = fork();
                    if (pid2 < 0)
                    {
                        perror("fork 2");
                        exit(EXIT_FAILURE);
                    }
                    if (pid2 == 0)
                    { // secondo figlio head -n 10
                        close(pip1[1]);
                        close(pip2[0]);

                        close(pip3[0]);
                        close(pip3[1]);

                        dup2(pip1[0], STDIN_FILENO);
                        close(pip1[0]);

                        dup2(pip2[1], STDOUT_FILENO);
                        close(pip2[1]);

                        execlp("head", "head", "-n", "10", NULL);
                        perror("head");
                        exit(EXIT_FAILURE);
                    }
                    int pid3 = fork();
                    if (pid3 < 0)
                    {
                        perror("fork 3");
                        exit(EXIT_FAILURE);
                    }
                    if (pid3 == 0)
                    { // terzo e ultimo figlio cut
                        close(pip1[0]);
                        close(pip1[1]);

                        close(pip2[1]);
                        close(pip3[0]);

                        dup2(pip2[0], STDIN_FILENO);
                        close(pip2[0]);

                        dup2(pip3[1], STDOUT_FILENO);
                        close(pip3[1]);

                        execlp("cut", "cut", "-f", "1,3,4", "-d", ",", NULL);
                        perror("cut");
                        exit(EXIT_FAILURE);
                    }
                    // CapoCnonessione
                    close(pip1[0]);
                    close(pip1[1]);
                    close(pip2[0]);
                    close(pip2[1]);

                    close(pip3[1]);

                    waitpid(pid1, NULL, 0);
                    waitpid(pid2, NULL, 0);
                    waitpid(pid3, NULL, 0);

                    char buffer[4096];
                    ssize_t bytes_read;

                    while ((bytes_read = read(pip3[0], buffer, sizeof(buffer))) > 0)
                    {
                        ssize_t bytes_written = write_all(ns, buffer, bytes_read);
                        if (bytes_written < 0)
                        {
                            perror("write_all on socket");
                            break;
                        }
                        write(ns, "\n", 1);
                    }
                    write_all(ns, "==fin==", sizeof("==fin=="));
                    write(ns, "\n", 1);
                    close(pip3[0]);
                }
                else
                {
                    puts("Autenticazione fallita o connessione terminata si richiesta del client \n");
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
