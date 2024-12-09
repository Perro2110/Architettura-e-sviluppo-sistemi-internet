#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "utils.h"
#include "rxb.h"

/* L'applicazione deve avere la seguente interfaccia: */
/* 	rstrlen hostname porta   */
/* 	argv[0] argv[1]  argv[2] */
int main(int argc, char **argv)
{
    int sd, err;
    struct addrinfo hints, *res, *ptr;
    char insUtente[4096];
    rxb_t rxb;
    char risposta[4096];
    size_t risposta_len;
    
    if(argc != 3) {
        fputs("Errore argomenti!\n", stderr);
        exit(EXIT_FAILURE);
    }
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    err = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (err != 0) {
        fprintf(stderr, "Errore gai: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    // Tentiamo la connessione con tutti gli indirizzi restituiti
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next) {
        sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sd < 0) continue;

        err = connect(sd, ptr->ai_addr, ptr->ai_addrlen);
        if (err == 0) break;

        close(sd);
    }

    if (ptr == NULL) {
        fputs("Errore connessione!\n", stderr);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    // Inizializza il buffer rxb una sola volta
    rxb_init(&rxb, 64 * 1024);

    while(1) {
        printf("Inserisci stringa da valutare, 'fine' per terminare\n");
        scanf("%s", insUtente);

        // Invia la stringa
        err = write_all(sd, insUtente, strlen(insUtente));
        if (err < 0) {
            fputs("Errore write!\n", stderr);
            exit(EXIT_FAILURE);
        }

        // Invia il newline
        err = write(sd, "\n", 1);
        if (err < 0) {
            fputs("Errore write!\n", stderr);
            exit(EXIT_FAILURE);
        }
        
        if (strcmp("fine", insUtente) == 0) {
            rxb_destroy(&rxb);
            close(sd);
            return 0;  // Uscita positiva
        }

        // Leggi la risposta usando rxb_readline
        memset(risposta, 0, sizeof(risposta));
        risposta_len = sizeof(risposta) - 1;
        
        err = rxb_readline(&rxb, sd, risposta, &risposta_len);
        if (err < 0) {
            fputs("Errore lettura risposta!\n", stderr);
            exit(EXIT_FAILURE);
        }

        // Stampa la risposta
        printf("Lunghezza stringa: %s\n", risposta);
    }

    // Cleanup (questo codice non verrà mai raggiunto nel programma attuale)
    rxb_destroy(&rxb);
    close(sd);
    return 0;
}