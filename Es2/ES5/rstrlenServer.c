// gcc -g -o ./eseS rstrlenServer.c rxb.c rxb.h utils.c utils.h
#include <stdint.h>
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h> // Per waitpid
#include "rxb.h"
#include "utils.h"

// server  porta
// argv[0] argv[1]
int main(int argc, char *argv[])
{
	int sd;			// Socket descriptor principale
	int err;		// Variabile per gestione errori
	int optval = 1; // Valore per setsockopt
	char *service;	// Porta del servizio
	struct addrinfo hints, *res;

	/* Controllo argomenti */
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\t%s port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/*
	 * Ignoriamo SIGPIPE per evitare che il processo termini se il client
	 * chiude la connessione mentre stiamo scrivendo
	 */
	signal(SIGPIPE, SIG_IGN);

	service = argv[1]; // Porta specificata come argomento

	/* Preparazione struttura hints per getaddrinfo */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;	 // Accetta sia IPv4 che IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;	 // Per server: bind su tutte le interfacce

	/* Otteniamo le informazioni sull'indirizzo */
	err = getaddrinfo(NULL, service, &hints, &res);
	if (err != 0)
	{
		fprintf(stderr, "Errore getaddrinfo: %s\n", gai_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* Creazione socket */
	sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sd < 0)
	{
		perror("Errore creazione socket");
		exit(EXIT_FAILURE);
	}

	/*
	 * Impostiamo SO_REUSEPORT per permettere il riutilizzo immediato
	 * della porta dopo la chiusura del server
	 */
	err = setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	if (err < 0)
	{
		perror("Errore setsockopt");
		exit(EXIT_FAILURE);
	}

	/* Associamo il socket all'indirizzo */
	err = bind(sd, res->ai_addr, res->ai_addrlen);
	if (err < 0)
	{
		perror("Errore bind");
		exit(EXIT_FAILURE);
	}

	/* Mettiamo il socket in ascolto */
	err = listen(sd, SOMAXCONN);
	if (err < 0)
	{
		perror("Errore listen");
		exit(EXIT_FAILURE);
	}

	printf("Server in ascolto sulla porta %s\n", service);

	/* Loop principale del server */
	while (1)
	{
		rxb_t rxb;		 // Buffer per la lettura
		int ns;			 // Socket per la nuova connessione
		char str1[1024]; // Buffer per la stringa ricevuta
		size_t str1_len; // Lunghezza della stringa

		/* Accettiamo una nuova connessione */
		ns = accept(sd, NULL, NULL);
		if (ns < 0)
		{
			if (errno == EINTR) // Interruzione da segnale
				continue;
			perror("Errore accept");
			exit(EXIT_FAILURE);
		}

		/* Creiamo un nuovo processo per gestire la connessione */
		int pid = fork();
		if (pid < 0)
		{
			perror("Errore fork");
			exit(EXIT_FAILURE);
		}
		if (pid == 0) // Processo figlio
		{
			close(sd); // Il figlio non usa il socket di ascolto

			/* Loop di gestione della connessione client */
			while (1)
			{
				/* Inizializzazione buffer di lettura */
				rxb_init(&rxb, 64 * 1024);
				memset(str1, 0, sizeof(str1));
				str1_len = sizeof(str1) - 1;

				/* Leggiamo una linea dal client */
				err = rxb_readline(&rxb, ns, str1, &str1_len);
				if (err < 0)
				{
					perror("Errore lettura");
					exit(EXIT_FAILURE);
				}

				/* Controlliamo se il client vuole terminare */
				int nn = strcmp(str1, "fine");
				printf("Ricevuto dal client: %s\n", str1);

				if (nn != 0) // Se non è "fine"
				{
					/* Calcoliamo la lunghezza della stringa */
					int num = strlen(str1);
					char stringa[250];
					sprintf(stringa, "%d", num);

					/* Inviamo la lunghezza al client */
					err = write_all(ns, stringa, strlen(stringa));
					if (err < 0)
					{
						fprintf(stderr, "Errore invio lunghezza\n");
						exit(EXIT_FAILURE);
					}

					/* Inviamo il carattere newline */
					err = write(ns, "\n", 1);
					if (err < 0)
					{
						fprintf(stderr, "Errore invio newline\n");
						exit(EXIT_FAILURE);
					}
				}
				else // Se riceviamo "fine"
				{
					printf("Client ha richiesto la chiusura\n");
					rxb_destroy(&rxb);
					close(ns);
					exit(0); // Terminiamo il processo figlio
				}
				/* Pulizia buffer di lettura */
				rxb_destroy(&rxb);
			}
		}
		/* Processo padre */
		close(ns); // Il padre chiude il socket della connessione
	}
	/* Chiusura socket principale (mai raggiunto nel codice attuale) */
	close(sd);
	return 0;
}