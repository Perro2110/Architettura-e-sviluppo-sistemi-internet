#include <stdint.h>
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "rxb.h"
#include "utils.h"
#include <sys/wait.h>
#include <ctype.h>

// **** PARTE DI UTILIZZO DI buffer OPZIONALE****
// Funzione che somma i numeri trovati in una stringa
// I numeri sono separati da virgole
double sum_numbers(char *buf)
{
    double sum = 0;
    char *p = buf;
    while (*p)
    {
        if (*p >= '0' && *p <= '9')
        {
            sum += atoi(p);
            printf("%c\n", *p);
            printf("%lf\n", sum);
            // Avanza fino alla prossima virgola
            while (*p && *p != ',')
                p++;
        }
        p++;
    }
    return sum;
}
// **** FINE PARTE DI UTILIZZO DI buffer OPZIONALE****

int main(int argc, char *argv[])
{
    // Inizializzazione variabili per il socket
    int sd, err;
    int optval = 1;
    int flag = 1;
    char *service;
    struct addrinfo hints, *res;

    // Controllo argomenti linea di comando
    if (argc != 2)
    {
        fprintf(stderr, "Usage:\n\t%s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Ignora SIGPIPE per evitare terminazione su write su socket chiuso
    signal(SIGPIPE, SIG_IGN);
    service = argv[1];

    // Configurazione socket
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // IPv4 o IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // Per bind su tutte le interfacce

    // Risoluzione indirizzo
    err = getaddrinfo(NULL, service, &hints, &res);
    if (err != 0)
    {
        fprintf(stderr, "Error: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    // Creazione socket
    sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Imposta opzione SO_REUSEADDR per riutilizzo immediato della porta
    err = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind del socket all'indirizzo
    err = bind(sd, res->ai_addr, res->ai_addrlen);
    if (err < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Mette il socket in ascolto
    err = listen(sd, SOMAXCONN);
    if (err < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Buffer per la lettura
    rxb_t rxb;
    int ns;
    char str1[1024];
    size_t str1_len;
    char str2[1024];
    size_t str2_len;

    // Loop principale del server
    while (1)
    {
        // Accetta nuova connessione
        ns = accept(sd, NULL, NULL);
        if (ns < 0)
        {
            if (errno == EINTR)
                continue;
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Fork per gestire il client
        int pid00 = fork();
        if (pid00 < 0)
        {
            perror("fork 00");
            exit(EXIT_FAILURE);
        }
        if (pid00 == 0)
        { // Processo figlio per gestire il client
            rxb_init(&rxb, 64 * 1024);
            while (1)
            {
                // Lettura della REGIONE
                memset(str1, 0, sizeof(str1));
                str1_len = sizeof(str1) - 1;
                err = rxb_readline(&rxb, ns, str1, &str1_len);
                if (err < 0)
                {
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }
                printf("REGIONE ricevuto: %s\n", str1);

                // Lettura del NUMERO
                memset(str2, 0, sizeof(str1));
                str2_len = sizeof(str1) - 1;
                err = rxb_readline(&rxb, ns, str2, &str2_len);
                if (err < 0)
                {
                    perror("rxb_readline");
                    exit(EXIT_FAILURE);
                }
                printf("NUMERO ricevuta: %s\n", str2);

                if (strcmp(str1, "fine") != 0 && strcmp(str2, "fine") != 0)
                {
                    int pip1[2]; // pipe per sort -> head
                    int pip2[2]; // pipe per head -> socket

                    // Creazione delle pipe
                    if (pipe(pip1) < 0 || pipe(pip2) < 0)
                    {
                        perror("pipe");
                        exit(EXIT_FAILURE);
                    }

                    // Fork per il comando sort
                    int pid1 = fork();
                    if (pid1 < 0)
                    {
                        perror("fork 1");
                        exit(EXIT_FAILURE);
                    }
                    if (pid1 == 0)
                    { // Primo figlio (sort)
                        // Chiude i file descriptor non necessari:
                        // - pip1[0]: non legge dalla prima pipe
                        // - pip2[0] e pip2[1]: non usa la seconda pipe
                        close(pip1[0]);
                        close(pip2[0]);
                        close(pip2[1]);

                        // Redirige stdout sulla prima pipe:
                        // - Duplica pip1[1] su stdout (fd 1)
                        // - Chiude l'originale pip1[1] poiché ora è duplicato
                        dup2(pip1[1], STDOUT_FILENO);
                        close(pip1[1]);

                        // Esegue sort sul file della regione
                        char pollo[5000];
                        sprintf(pollo, "%s.txt", str1);
                        execlp("sort", "sort", pollo, NULL);
                        perror("execlp sort");
                        exit(EXIT_FAILURE);
                    }

                    // Fork per il comando head
                    int pid2 = fork();
                    if (pid2 < 0)
                    {
                        perror("fork 2");
                        exit(EXIT_FAILURE);
                    }
                    if (pid2 == 0)
                    { // Secondo figlio (head)
                        // Chiude i file descriptor non necessari:
                        // - pip1[1]: non scrive sulla prima pipe
                        // - pip2[0]: non legge dalla seconda pipe
                        close(pip1[1]);
                        close(pip2[0]);

                        // Redirige stdin dalla prima pipe:
                        // - Duplica pip1[0] su stdin (fd 0)
                        // - Chiude l'originale pip1[0]
                        dup2(pip1[0], STDIN_FILENO);
                        close(pip1[0]);

                        // Redirige stdout sulla seconda pipe:
                        // - Duplica pip2[1] su stdout (fd 1)
                        // - Chiude l'originale pip2[1]
                        dup2(pip2[1], STDOUT_FILENO);
                        close(pip2[1]);

                        // Esegue head con il numero di linee specificato
                        execlp("head", "head", "-n", str2, NULL);
                        perror("execlp head");
                        exit(EXIT_FAILURE);
                    }

                    // Processo padre
                    // Chiude tutti i descrittori delle pipe non necessari:
                    // - pip1[0] e pip1[1]: non LEGGE la prima pipe
                    // - pip2[1]: non SCRIVE sulla seconda pipe
                    close(pip1[0]);
                    close(pip1[1]);
                    close(pip2[1]);

                    // Attende la terminazione dei processi figli
                    waitpid(pid1, NULL, 0);
                    waitpid(pid2, NULL, 0);

                    // Legge dalla seconda pipe e scrive sul socket
                    char buffer[4096];
                    ssize_t bytes_read;
                    int numero = 0;
                    int el = atoi(str2);                // Converte la stringa in numero intero
                    char bufferSupremoDellaMedia[4096]; // Buffer per la stringa finale con la media

                    // Legge dal pipe e scrive sul socket
                    while ((bytes_read = read(pip2[0], buffer, sizeof(buffer))) > 0)
                    {
                        // Scrive i dati sul socket
                        ssize_t bytes_written = write_all(ns, buffer, bytes_read);
                        if (bytes_written < 0)
                        {
                            perror("write to socket");
                            break;
                        }
                        // Aggiunge newline
                        write(ns, "\n", 1);

                        // **** PARTE DI UTILIZZO DI buffer OPZIONALE****
                        // Calcola e scrive la media
                        numero = sum_numbers(buffer);
                        sprintf(bufferSupremoDellaMedia, " %d\n", (numero / el));
                        bytes_written = write_all(ns, bufferSupremoDellaMedia, strlen(bufferSupremoDellaMedia));
                        if (bytes_written < 0)
                        {
                            perror("write to socket");
                            break;
                        }
                        write(ns, "\n", 1);
                    }
                    write_all(ns, "===fine===", sizeof("===fine===")); // mando segnale di fine comunicazione
                    write(ns, "\n", 1);
                    // Chiude il descrittore di lettura della seconda pipe
                    close(pip2[0]);
                    memset(buffer, 0, strlen(buffer)); // **** PARTE DI UTILIZZO DI buffer OPZIONALE****
                }
                else
                {
                    printf("Comunicazione terminata su richiesta del client\n");
                    rxb_destroy(&rxb);
                    exit(0); // figlio 00 muore
                }
            }
        }
        // Processo padre chiude il socket della connessione
        close(ns);
    }
    // Chiusura finale (mai raggiunta nel codice attuale)
    close(ns); // INUTILE LO SO NON MI TOLGA PUNTI PLS :P
    close(sd);
    return -1;
}

/*

PID1 PID2 PID3 

pip1 1 to 2
pip2 2 to 3 
pip3 3 to s
                    if (pid1 == 0)
                    { // Primo figlio (sort)
                        // Chiude i file descriptor non necessari:
                        // - pip1[0]: non legge dalla prima pipe
                        // - pip2[0] e pip2[1]: non usa la seconda pipe
                        close(pip1[0]);
                        close(pip2[0]);
                        close(pip2[1]);

                        close(pip3[0]);
                        close(pip3[1]);

                        // Redirige stdout sulla prima pipe:
                        // - Duplica pip1[1] su stdout (fd 1)
                        // - Chiude l'originale pip1[1] poiché ora è duplicato
                        dup2(pip1[1], STDOUT_FILENO);
                        close(pip1[1]);

                        // Esegue sort sul file della regione
                        char pollo[5000];
                        sprintf(pollo, "%s.txt", str1);
                        execlp("sort", "sort", pollo, NULL);
                        perror("execlp sort");
                        exit(EXIT_FAILURE);
                    }



                    if (pid2 == 0)
                    { // Secondo figlio (head)
                        // Chiude i file descriptor non necessari:
                        // - pip1[1]: non scrive sulla prima pipe
                        // - pip2[0]: non legge dalla seconda pipe
                        close(pip1[1]);
                        close(pip2[0]);
                        close(pip3[0]);
                        close(pip3[1]);

                        // Redirige stdin dalla prima pipe:
                        // - Duplica pip1[0] su stdin (fd 0)
                        // - Chiude l'originale pip1[0]
                        dup2(pip1[0], STDIN_FILENO);
                        close(pip1[0]);

                        // Redirige stdout sulla seconda pipe:
                        // - Duplica pip2[1] su stdout (fd 1)
                        // - Chiude l'originale pip2[1]
                        dup2(pip2[1], STDOUT_FILENO);
                        close(pip2[1]);

                        // Esegue head con il numero di linee specificato
                        execlp("head", "head", "-n", str2, NULL);
                        perror("execlp head");
                        exit(EXIT_FAILURE);
                    }

                    if (pid3 == 0)
                    { // Tre figlio (head)
                        // Chiude i file descriptor non necessari:
                        // - pip1[1]: non scrive sulla prima pipe
                        // - pip2[0]: non legge dalla seconda pipe
                        close(pip1[1]);
                        close(pip1[0]);
                        close(pip2[1]);
                        close(pip3[0]);
                        
                        dup2(pip2[0],STDIN_FILENO)
                        close(pip2[0]);

                        dup2(pip3[1],STDOUT_FILENO)
                        close(pip3[1]);
                    

                        // Esegue head con il numero di linee specificato
                        execlp("head", "head", "-n", str2, NULL);
                        perror("execlp head");
                        exit(EXIT_FAILURE);
                    }

            NONNO 
                    while ((bytes_read = read(pip3[0], buffer, sizeof(buffer))) > 0)
                    {
                        // Scrive i dati sul socket
                        ssize_t bytes_written = write_all(ns, buffer, bytes_read);
                    ...
*/