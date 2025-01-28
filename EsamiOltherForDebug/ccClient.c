#define _POSIX_C_SOURCE 200809L
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rxb.h"
#include "utils.h"

#define MAX_REQUEST_SIZE 4096

int main(int argc, char *argv[]){
    int sd, err;
    struct addrinfo hints, *res, *ptr;
    char *server, *porta;
    rxb_t rxb;

    if (argc != 3){
        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    server = argv[1];
    porta = argv[2];

    err = getaddrinfo(server, porta, &hints, &res);
    if (err){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next){
        sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sd < 0)
            continue;
        err = connect(sd, ptr->ai_addr, ptr->ai_addrlen);
        if (err == 0)
            break;
        close(sd);
    }
    if (ptr == NULL){
        fprintf(stderr, "connection failed\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    rxb_init(&rxb, MAX_REQUEST_SIZE * 2);

    while(1){
        char username[1024];
        char password[1024];
        char categoria[1024];

        puts("Username:");
        fgets(username, sizeof(username), stdin);

        if (strcmp(username, "fine\n") == 0){
            puts("Termino");
            break;
        }

        puts("Password:");
        fgets(password, sizeof(password), stdin);

        if (strcmp(password, "fine\n") == 0){
            puts("Termino");
            break;
        }

        puts("Inserisci la categoria da visualizzare:");
        fgets(categoria, sizeof(categoria), stdin);
        if (strcmp(categoria, "fine\n") == 0){
            puts("Termino");
            break;
        }

        err = write_all(sd, username, strlen(username));
        if (err < 0){
            perror("write_all");
            exit(EXIT_FAILURE);
        }

        err = write_all(sd, password, strlen(password));
        if (err < 0){
            perror("write_all");
            exit(EXIT_FAILURE);
        }

        err = write_all(sd, categoria, strlen(categoria));
        if (err < 0){
            perror("write_all");
            exit(EXIT_FAILURE);
        }

        while(1){
            char line[4096];
            size_t line_len;

            memset(line, 0, sizeof(line));
            line_len = sizeof(line)-1;
            err = rxb_readline(&rxb, sd, line, &line_len);
            if (err < 0){
                fputs("Errore rxb_readline", stderr);
                rxb_destroy(&rxb);
                close(sd);
                exit(EXIT_FAILURE);
            }
#ifdef HAVE_LIBUNISTRING
			if (u8_check((uint8_t *)line, line_len) != NULL) {
				fputs("Errore: testo UTF-8 invalido!", stderr);
				rxb_destroy(&rxb);
				close(sd);
				exit(EXIT_FAILURE);
			}
#endif

            if (strcmp(line, "---FINE RICHIESTA---") == 0)
                break;
            puts(line);
        }
        
    }
    rxb_destroy(&rxb);
    close(sd);
    
    return 0;
}