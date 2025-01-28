// gcc -g -o .eseS server.c rxb.c rxb.h utils.c utils.h
// inizio 18.01 18.34
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
#include <errno.h>
#include <stdint.h>

int main(int argc, char const *argv[])
{
    int sd, err;
    int optval = 1;
    int flag = 1;
    char *service;
    struct addrinfo hints, *res;

    if (argc != 2)
    {
        fprintf(stderr, "errore num argomenti");
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
        perror("errore gettadrrinfo");
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
            perror("acept");
            exit(EXIT_FAILURE);
        }
        int pid00 = fork();
        if (pid00 < 0)
        {
            perror("fork 00");
            exit(EXIT_FAILURE);
        }
        if (pid00 == 0)
        { // PROCESSO CAPO CONNESSIONE
            rxb_init(&rxb, 64 * 1024);
            while (1)
            {

                // username
                memset(str1, 0, sizeof(str1));
                str1_len = sizeof(str1) - 1;
                err = rxb_readline(&rxb, ns, str1, &str1_len);
                if (err < 0)
                {
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }
                // nomeProg
                memset(str2, 0, sizeof(str2));
                str2_len = sizeof(str2) - 1;
                err = rxb_readline(&rxb, ns, str2, &str2_len);
                if (err < 0)
                {
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }

                // statoProg
                memset(str3, 0, sizeof(str3));
                str3_len = sizeof(str3) - 1;
                err = rxb_readline(&rxb, ns, str3, &str3_len);
                if (err < 0)
                {
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }

                if (strcmp(str1, "fine") != 0 && strcmp(str2, "fine") != 0 && strcmp(str3, "fine") != 0)
                {
                    int pip1[2]; // grep -> grep
                    int pip2[2]; // grep -> socket

                    if (pipe(pip1) < 0 || pipe(pip2) < 0)
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
                    { // primo figlio (grep nome_utente)
                        close(pip1[0]);
                        close(pip2[0]);
                        close(pip2[1]);

                        dup2(pip1[1], STDOUT_FILENO);
                        close(pip1[1]);

                        char buffy[5000];
                        sprintf(buffy,"%s.txt",str2);
                        execlp("grep","grep",str1,buffy,NULL);
                        perror("excelp grep1");
                        exit(EXIT_FAILURE);
                    }

                    int pid2 = fork();
                    if (pid2 < 0)
                    {
                        perror("fork 2");
                        exit(EXIT_FAILURE);
                    }
                    if (pid2 == 0)
                    { // secondo figlio (grep nome_versione)
                        close(pip1[1]);
                        close(pip2[0]);
                        
                        dup2(pip1[0],STDIN_FILENO);
                        close(pip1[0]);

                        dup2(pip2[1],STDOUT_FILENO);
                        close(pip2[1]);

                        execlp("grep","grep",str3,NULL);
                        perror("excelp grep2");
                        exit(EXIT_FAILURE);
                    }
                    // Processo capo connessione
                    close(pip1[0]);
                    close(pip1[1]);
                    close(pip2[1]);

                    waitpid(pid1,NULL,0);
                    waitpid(pid2,NULL,0);

                    char buffer[4096];
                    ssize_t bytes_read;

                    while((bytes_read=read(pip2[0],buffer,sizeof(buffer))) > 0){
                        ssize_t bytes_written=write_all(ns,buffer,bytes_read);
                        if(bytes_written < 0){
                            perror("write to socket");
                            break;
                        }
                        write(ns,"\n",1);
                    }
                    write_all(ns,"==fin==",sizeof("==fin=="));
                    write(ns,"\n",1);
                    close(pip2[0]);
                }else{
                    printf("Comunicazione terminata su richiesta del client \n");
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
