#define _POSIX_C_SOURCE 200809L
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include "rxb.h"
#include "utils.h"

#define MAX_REQUEST_SIZE 4096

void sigchld_handler(int signo)
{
    int status;

    (void)signo;

    while (waitpid(-1, &status, WNOHANG) > 0)
        continue;
}

int autorizza(const char *username, const char *password)
{
    return 1;
}

int main(int argc, char *argv[])
{
    int err, sd, on;
    struct sigaction sa;
    struct addrinfo hints, *res;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
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

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    err = getaddrinfo(NULL, argv[1], &hints, &res);
    if (err != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    on = 1;
    err = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
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

    freeaddrinfo(res);

    while (1)
    {
        int ns, pid;

        ns = accept(sd, NULL, NULL);
        if (ns < 0)
        {
            perror("accept");
            continue;
        }

        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0)
        {
            rxb_t rxb;

            rxb_init(&rxb, MAX_REQUEST_SIZE * 2);
            signal(SIGCHLD, SIG_DFL);
            char buffer[5000];
            while (1)
            {
                int pip1[2], pip2[2], pip3[3];

                if (pipe(pip1) < 0 || pipe(pip2) < 0 || pipe(pip3) < 0)
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                int pid1, pid2, pid3;
                char username[1024];
                char password[1024];
                char categoria[1024];
                size_t username_len, password_len, categoria_len;
                int status;
                char *end_request = "---FINE RICHIESTA---\n";

                memset(username, 0, sizeof(username));
                username_len = sizeof(username) - 1;

                err = rxb_readline(&rxb, ns, username, &username_len);
                if (err < 0)
                {
                    fputs("Errore rxb_readline!", stderr);
                    rxb_destroy(&rxb);
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#ifdef HAVE_LIBUNISTRING
                if (u8_check((uint8_t *)username, username_len) != NULL)
                {
                    fputs("Testo non UTF-8 valido!", stderr);
                    rxb_destroy(&rxb);
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif

                memset(password, 0, sizeof(password));
                password_len = sizeof(password) - 1;

                err = rxb_readline(&rxb, ns, password, &password_len);
                if (err < 0)
                {
                    fputs("Errore rxb_readline!", stderr);
                    rxb_destroy(&rxb);
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#ifdef HAVE_LIBUNISTRING
                if (u8_check((uint8_t *)password, password_len) != NULL)
                {
                    fputs("Testo non UTF-8 valido!", stderr);
                    rxb_destroy(&rxb);
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif
                memset(categoria, 0, sizeof(categoria));
                categoria_len = sizeof(categoria) - 1;

                err = rxb_readline(&rxb, ns, categoria, &categoria_len);
                if (err < 0)
                {
                    fputs("Errore rxb_readline!", stderr);
                    rxb_destroy(&rxb);
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#ifdef HAVE_LIBUNISTRING
                if (u8_check((uint8_t *)categoria, categoria_len) != NULL)
                {
                    fputs("Testo non UTF-8 valido!", stderr);
                    rxb_destroy(&rxb);
                    close(ns);
                    exit(EXIT_FAILURE);
                }
#endif

                // puts(categoria);

                if (autorizza(username, password))
                {
                    puts("Autorizzato");
                    fflush(stdout);
                }
                else
                {
                    puts("Non autorizzato");
                    exit(EXIT_FAILURE);
                }

                pid1 = fork();
                if (pid1 < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                if (pid1 == 0)
                {
                    // Nipote
                    char filename[1024];

                    snprintf(filename, sizeof(filename), "%s.txt", categoria);
                    puts(filename);
                    
                    close(pip1[0]);
                    close(pip2[0]);
                    close(pip2[1]);

                    close(pip3[0]);
                    close(pip3[1]);

                    dup2(pip1[1],STDOUT_FILENO);
                    close(pip1[1]);

                    execlp("sort", "sort", "-n",filename, NULL);
                    perror("execlp");
                    exit(EXIT_FAILURE);
                }

                pid2 = fork();
                if (pid2 < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                if (pid2 == 0)
                {
                    close(pip1[1]);
                    close(pip2[0]);
                    close(pip3[0]);
                    close(pip3[1]);

                    dup2(pip1[0],STDIN_FILENO);
                    close(pip1[0]);

                    dup2(pip2[1],STDOUT_FILENO);
                    close(pip2[1]);


                    execlp("head", "head", "-n", "10", NULL);
                    perror("execlp");
                    exit(EXIT_FAILURE);
                }

                pid3 = fork();
                if (pid3 < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                if (pid3 == 0)
                {
                    // Nipote 3

                    close(pip1[1]);
                    close(pip1[0]);
                    close(pip2[1]);
                    close(pip3[0]);

                    dup2(pip2[0],STDIN_FILENO);
                    close(pip2[0]);

                    dup2(pip3[1],STDOUT_FILENO);
                    close(pip3[1]);

                    execlp("cut", "cut", "-f", "1,3,4", "-d", ",",NULL);

                    perror("cut");
                    exit(EXIT_FAILURE);
                }

                close(pip1[0]);
                close(pip1[1]);
                close(pip2[0]);
                close(pip2[1]);
                close(pip3[1]);

                waitpid(pid1, &status, 0);
                waitpid(pid2, &status, 0);
                waitpid(pid3, &status, 0);
                ssize_t bytes_read;
               while((bytes_read = read (pip3[0],buffer,sizeof(buffer)))>0)
               {
                    ssize_t bytes_written = write_all(ns,buffer,bytes_read);
                    if(bytes_written <0){
                        perror("wraal");
                        exit(EXIT_FAILURE);
                    }
                    write(ns,"\n",1);
               }
               write_all(ns,"---FINE RICHIESTA---",sizeof("---FINE RICHIESTA---"));
               write(ns,"\n",1);
            }

            exit(0);
        }

        close(ns);
    }

    close(sd);

    return 0;
}