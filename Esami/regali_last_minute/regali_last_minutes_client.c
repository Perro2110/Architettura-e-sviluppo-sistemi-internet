// inizio ore 16.15 16.43
// gcc -g -o ./eseC client.c rxb.c rxb.h utils.c utils.h

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "utils.h"
#include "rxb.h"

int main(int argc, char const *argv[])
{
    int sd, err, nread;
    uint8_t buffer[4096];
    size_t buffer_len;
    rxb_t rxb;

    char insUtente[4096];
    struct addrinfo hints, *res, *ptr;

    if (argc < 3)
    {
        fputs("Errore numero argomenti", stderr);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    err = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (err != 0)
    {
        fprintf(stderr, "Errore gai_ %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sd < 0)
            continue;
        err = connect(sd, ptr->ai_addr, ptr->ai_addrlen);
        if (err == 0)
            break;
        close(sd);
    }

    if (ptr == NULL)
    {
        fputs("Errore connessione", stderr);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    rxb_init(&rxb,64*1024); // ATTENTO MARCO !!

    while (1)
    {
        printf("Inserire categoria dei regali, 'fine' per finire \n");
        scanf("%s", &insUtente);
        /*  Mando al server */
        err = write_all(sd, insUtente, strlen(insUtente));
        if (err < 0)
        {
            fputs("Errore write_all 1", stderr);
            exit(EXIT_FAILURE);
        }
        err = write(sd, "\n", 1);
        if (err < 0)
        {
            fputs("Errore write 1", stderr);
            exit(EXIT_FAILURE);
        }
        if (strcmp("fine", insUtente) == 0)
            return 1;

        /** nome del produttore dei regali: */
        printf("Inserire nome del produttore dei regali, 'fine' per finire \n");
        scanf("%s", &insUtente);
        /*  Mando al server */
        err = write_all(sd, insUtente, strlen(insUtente));
        if (err < 0)
        {
            fputs("Errore write_all 2", stderr);
            exit(EXIT_FAILURE);
        }
        err = write(sd, "\n", 1);
        if (err < 0)
        {
            fputs("Errore write 2", stderr);
            exit(EXIT_FAILURE);
        }
        if (strcmp("fine", insUtente) == 0)
            return 1;

        /** Ordinamento type dei regali: */
        printf("Inserire modalità di ordinamento dei regali, 'crescente' o 'decrescente', fine per finire: \n");
        scanf("%s", &insUtente);
        /*  Mando al server */
        err = write_all(sd, insUtente, strlen(insUtente));
        if (err < 0)
        {
            fputs("Errore write_all 3", stderr);
            exit(EXIT_FAILURE);
        }
        err = write(sd, "\n", 1);
        if (err < 0)
        {
            fputs("Errore write 3", stderr);
            exit(EXIT_FAILURE);
        }
        if (strcmp("fine", insUtente) == 0)
            return 1;

        // Ricezzione
        for (;;)
        {
            memset(buffer, 0, sizeof(buffer));
            buffer_len = sizeof(buffer) - 1;
            
            if (rxb_readline(&rxb, sd, buffer, &buffer_len) < 0)
            {
                rxb_destroy(&rxb);
                fprintf(stderr, "Connessione chiusa dal server \n");
                close(sd);
                exit(EXIT_FAILURE);
            }
            

#ifdef USE_LIBUNISTRING
            if (u8_check((uint8_t *)buffer, buffer_len) != NULL)
            {
                fprintf(stderr, "Response is not in UTF-8 \n");
                close(sd);
                exit(EXIT_FAILURE);
            }
#endif

            puts(buffer);
            if (strcmp(buffer, "==fin==") == 0)
            {
                break;
            }
        }
    }
    rxb_destroy(&rxb);
    close(sd);
    return 0;
}
