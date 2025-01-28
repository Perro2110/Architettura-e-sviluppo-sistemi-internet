// gcc -g -o .eseC client.c rxb.c rxb.h utils.c utils.h
//        17.35 18.01    |--> 26 min  
//        18.01 18.34    |--> 33 min
//        18.34 18.55    |--> 21 min = 80 min --> 1.20 h   
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
        fputs("Errore numero argomenti !!", stderr);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    err = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (err != 0)
    {
        fprintf(stderr, "errore gettaddrinfo");
        exit(EXIT_FAILURE);
    }
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    { // FALLBACK
        sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sd < 0)
            continue;

        err = connect(sd, ptr->ai_addr, ptr->ai_addrlen);
        if (err == 0)
            break;

        close;
    }
    if (ptr == NULL)
    {
        fputs("Errore connessione!", stderr);
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);
    rxb_init(&rxb, 64 * 1024);
    while (1)
    {
        /** INVIO:  */
        /** username */
        printf("Inserire username, 'fine' per terminare\n");
        scanf("%s", &insUtente);
        /* Mando e scrivo stringa in UTF-8*/
        err = write_all(sd, insUtente, strlen(insUtente));
        if (err < 0)
        {
            fputs("Errore write_all num_1", stderr);
            exit(EXIT_FAILURE);
        }
        err = write(sd, "\n", 1);
        if (err < 0)
        {
            fputs("Errore write num_1", stderr);
            exit(EXIT_FAILURE);
        }
        if (strcmp("fine", insUtente) == 0)
            return 1;

        /** nome progetto sw */
        printf("Inserire nome progetto software, 'fine' per terminare\n");
        scanf("%s", &insUtente);
        /* Mando e scrivo stringa in UTF-8*/
        err = write_all(sd, insUtente, strlen(insUtente));
        if (err < 0)
        {
            fputs("Errore write_all num_2", stderr);
            exit(EXIT_FAILURE);
        }
        err = write(sd, "\n", 1);
        if (err < 0)
        {
            fputs("Errore write num_2", stderr);
            exit(EXIT_FAILURE);
        }
        if (strcmp("fine", insUtente) == 0)
            return 1;

        /** nome della versione */
        printf("Inserire nome della versione, 'fine' per terminare\n");
        scanf("%s", &insUtente);
        /* Mando e scrivo stringa in UTF-8*/
        err = write_all(sd, insUtente, strlen(insUtente));
        if (err < 0)
        {
            fputs("Errore write_all num_3", stderr);
            exit(EXIT_FAILURE);
        }
        err = write(sd, "\n", 1);
        if (err < 0)
        {
            fputs("Errore write_all num_3", stderr);
            exit(EXIT_FAILURE);
        }
        if (strcmp("fine", insUtente) == 0)
            return 1;

        /** RICEZZIONE: */
        for (;;)
        {
            memset(buffer, 0, sizeof(buffer));
            buffer_len = sizeof(buffer) - 1;
            if (rxb_readline(&rxb, sd, buffer, &buffer_len) < 0)
            {
                rxb_destroy(&rxb);
                fprintf(stderr, "Connessione chiusa dal server\n");
                close(sd);
                exit(EXIT_FAILURE);
            }
#ifdef USE_LIBUNISTRING
            if (u8_check((uint8_t *)buffer, buffer_len) != NULL)
            {
                fprintf(stderr, "Response is not valis UTF-8!\n");
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
