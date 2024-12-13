// inizio c: 8.35 fine c 9.57 fine java 10.30 

// gcc -g -o ./eseC client.c rxb.c rxb.h utils.c utils.h
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include "utils.h"
#include "rxb.h"
#include <string.h>

/*  eseC     server porta*/
/*  argv[0] argv[1] argv[2]*/
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
        fputs("Errore argomenti", stderr);
        exit(EXIT_FAILURE);
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    err = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (err != 0)
    {
        fprintf(stderr, "Errore gai: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        // Fallback
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
        fputs("Errore  connessione!", stderr);
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);
    rxb_init(&rxb, 64 * 1024);

    while (1)
    {
        // Username
        printf("\n Inserire USERNAME, 'fine' per terminare \n");
        scanf("%s", &insUtente);
        err = write_all(sd, insUtente, strlen(insUtente));
        if (err < 0)
        {
            fputs("Errore write Username1", stderr);
            exit(EXIT_FAILURE);
        }
        err = write(sd, "\n", 1);
        if (err < 0)
        {
            fputs("Errore write Username2", stderr);
            exit(EXIT_FAILURE);
        }
        if (strcmp("fine", insUtente) == 0)
        {
            return 1; // uscita positiva
        }

        // Password
        printf("\n Inserire PASSWORD, 'fine' per terminare \n");
        scanf("%s", &insUtente);
        err = write_all(sd, insUtente, strlen(insUtente));
        if (err < 0)
        {
            fputs("Errore write PASSWORD1", stderr);
            exit(EXIT_FAILURE);
        }
        err = write(sd, "\n", 1);
        if (err < 0)
        {
            fputs("Errore write PASSWORD2", stderr);
            exit(EXIT_FAILURE);
        }
        if (strcmp("fine", insUtente) == 0)
        {
            return 1; // uscita positiva
        }
        // Macchinario
        printf("\n Inserire Macchinario, 'fine' per terminare \n");
        scanf("%s", &insUtente);
        err = write_all(sd, insUtente, strlen(insUtente));
        if (err < 0)
        {
            fputs("Errore write Macchinario1", stderr);
            exit(EXIT_FAILURE);
        }
        err = write(sd, "\n", 1);
        if (err < 0)
        {
            fputs("Errore write Macchinario2", stderr);
            exit(EXIT_FAILURE);
        }
        if (strcmp("fine", insUtente) == 0)
        {
            return 1; // uscita positiva
        }
        // RICEZZIONE
        for (;;)
        {
            memset(buffer, 0, sizeof(buffer));
            buffer_len = sizeof(buffer) - 1;
            if (rxb_readline(&rxb, sd, buffer, &buffer_len) < 0)
            {
                rxb_destroy(&rxb);
                fprintf(stderr, "Connessione chiusa dal server");
                close(sd);
                exit(EXIT_FAILURE);
            }
#ifdef USE_LIBUNISTRING
            if (u8_check((u_int8_t *)buffer, buffer_len) != NULL)
            {
                fprintf(stderr, "Response is not valid UTF-8!! \n");
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
