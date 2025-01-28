// 16.38 17.12
// gcc -g -o ./eseS server.c rxb.c rxb.h utils.c utils.h
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include "utils.h"
#include "rxb.h"

#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <ctype.h>

void sigchld_handler(int signo)
{
    int status;
    (void)signo;
    while (waitpid(-1, &status, WNOHANG) > 0)
        continue;
}

int main(int argc, char const *argv[])
{
    int sd, err;
    int optval = 1;
    int flag = 1;
    char *service;
    struct addrinfo hints, *res;
    struct sigaction sa;

    if (argc != 2)
    {
        fprintf(stderr, "Errore numero argomenti");
        exit(EXIT_FAILURE);
    }
    signal(SIGPIPE, SIG_IGN);

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sigchld_handler;
    err = sigaction(SIGCHLD, &sa, NULL);
    if (err < 0)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    service = argv[1];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    err = getaddrinfo(NULL, service, &hints, &res);
    if (err != 0)
    {
        fprintf(stderr, "Errore gettaddrinfo");
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
        { // Figlio capo connessione
            rxb_init(&rxb, 64 * 1024);
            signal(SIGCHLD, SIG_DFL);

            while (1)
            {
                // MESE
                memset(str1, 0, sizeof(str1));
                str1_len = sizeof(str1) - 1;

                err = rxb_readline(&rxb, ns, str1, &str1_len);
                if (err < 0)
                {
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }

                // tipologia
                memset(str2, 0, sizeof(str2));
                str2_len = sizeof(str2) - 1;

                err = rxb_readline(&rxb, ns, str2, &str2_len);
                if (err < 0)
                {
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }

                // localita
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
                    int pip1[2]; // sort -> grep -> socket
                    if(pipe(pip1) < 0){
                        perror("pipe");
                        exit(EXIT_FAILURE);
                    }
                    int pid1=fork();
                    if(pid1 <0)
                    {
                        perror("fork 1");
                        exit(EXIT_FAILURE);
                    }
                    if(pid1==0)
                    {
                        close(pip1[0]);
                        close(ns);

                        dup2(pip1[1],STDOUT_FILENO);
                        close(pip1[1]);
                    
                        char filename[5000];
                        snprintf(filename,sizeof(filename),"%s/%s.txt",str2,str1);
                        execlp("sort","sort","-n","-r",filename,NULL);
                        perror("sort");
                        exit(EXIT_FAILURE);
                    }
                    int pid2=fork();
                    if(pid2<0){
                        perror("fork2");
                        exit(EXIT_FAILURE);
                    }
                    if(pid2==0)
                    {
                        close(pip1[1]);
                        
                        dup2(pip1[0],STDIN_FILENO);
                        close(pip1[0]);

                        dup2(ns,STDOUT_FILENO);
                        close(ns);

                        execlp("grep","grep",str3,NULL);
                        perror("grep");
                        exit(EXIT_FAILURE);
                    }
                    close(pip1[0]);
                    close(pip1[1]);

                    waitpid(pid1,NULL,0);
                    waitpid(pid2,NULL,0);
                    
                    err = write(ns,"\n",1);
                    if(err < 0)
                    {
                        perror("write new line 1");
                        exit(EXIT_FAILURE);
                    }
                    err = write_all(ns,"==fin==",sizeof("==fin=="));
                                        if(err < 0)
                    {
                        perror("write ==fin==");
                        exit(EXIT_FAILURE);
                    }
                    err = write(ns,"\n",1);
                    if(err < 0)
                    {
                        perror("write new line 2");
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
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
