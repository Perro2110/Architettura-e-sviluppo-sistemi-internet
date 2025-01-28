// 17.15 17.38
//  gcc -g -o ./eseC client.c rxb.c rxb.h utils.c utils.h
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "utils.h"
#include "rxb.h"

#include <signal.h>

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
        fputs("Errore numero di argomenti", stderr);
        exit(EXIT_FAILURE);
    }
    signal(SIGPIPE, SIG_IGN);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    err = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (err != 0)
    {
        fprintf(stderr, "Errore, getaddrinfo");
        exit(EXIT_FAILURE);
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    { // FALL BACK
        sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if(sd < 0)
            continue;
        err = connect(sd,ptr->ai_addr,ptr->ai_addrlen);
        if(err == 0)
            break;
        
        close(sd);
    }
    if(ptr == NULL)
    {
        fputs("Errore connessione",stderr);
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);
    rxb_init(&rxb,64*1024);

    while(1)
    {
        printf("\n Inserire nome_giocatore di interesse, 'fine' per terminare \n");
        scanf("%s",&insUtente);
        err = write_all(sd,insUtente,strlen(insUtente));
        if(err < 0)
        {
            fputs("Errore write_all",stderr);
            exit(EXIT_FAILURE);
        }
        err = write(sd,"\n",1);
        if(err < 0)
        {
            fputs("Errore write",stderr);
            exit(EXIT_FAILURE);
        }

        if(strcmp("fine",insUtente)==0)
            return 1;
        
        printf("\n Inserire nome_squadra di interesse, 'fine' per terminare \n");
        scanf("%s",&insUtente);
        err = write_all(sd,insUtente,strlen(insUtente));
        if(err < 0)
        {
            fputs("Errore write_all",stderr);
            exit(EXIT_FAILURE);
        }
        err = write(sd,"\n",1);
        if(err < 0)
        {
            fputs("Errore write",stderr);
            exit(EXIT_FAILURE);
        }

        if(strcmp("fine",insUtente)==0)
            return 1;
        
        printf("\n Inserire edizione di interesse, 'fine' per terminare \n");
        scanf("%s",&insUtente);
        err = write_all(sd,insUtente,strlen(insUtente));
        if(err < 0)
        {
            fputs("Errore write_all",stderr);
            exit(EXIT_FAILURE);
        }
        err = write(sd,"\n",1);
        if(err < 0)
        {
            fputs("Errore write",stderr);
            exit(EXIT_FAILURE);
        }

        if(strcmp("fine",insUtente)==0)
            return 1;
        
        for(;;)
        {
            memset(buffer,0,sizeof(buffer));
            buffer_len=sizeof(buffer)-1;
            if(rxb_readline(&rxb,sd,buffer,&buffer_len)<0)
            {
                rxb_destroy(&rxb);
                fprintf(stderr,"Connessione chiusa dal server \n");
                close(sd);
                exit(EXIT_FAILURE);
            }
#ifdef USE_LIBUNISTRING
            if(u8_check((uint8_t *)buffer,buffer_len) != NULL)
            {
                fprintf(stderr,"Risposta non in formato UTF-8 \n");
                close(sd);
                exit(EXIT_FAILURE);
            }
#endif
            puts(buffer);
            if(strcmp(buffer,"==fin==")==0)
            {
                break;
            }
        }
    }
    rxb_destroy(&rxb);
    close(sd);
    return 0;
}
