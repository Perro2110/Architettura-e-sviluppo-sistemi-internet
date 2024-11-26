/*
Si realizzi, utilizzando la Socket API, un'applicazione distribuita
Client/Server che permetta a un ristoratore di verificare la disponibilità di
vini pregiati nella propria cantina.

Il Client deve presentare la seguente interfaccia:

	verifica_disponibilità_vini server porta

dove server e porta sono rispettivamente il nome logico dell'host e il numero
di porta su cui si trova il Server.

Per prima cosa, il Client si interfaccia con l'utente, da cui riceve, via
tastiera, il nome del vino e l'annata di interesse. Il Client dovrà inviare i
dati inseriti al Server, che a sua volta dovrà occuparsi di analizzare i
registri del magazzino per reperire le informazioni sui vini richiesti
dall'utente. Infine, il Server dovrà inviare le informazioni al Client, che le
stamperà a video.

Si supponga che le informazioni sul magazzino del ristorante siano salvate nel
file /var/local/magazzino.txt all'interno del filesystem del Server. In
particolare, si supponga che ogni riga del file contenga i dati di una
particolare tipologia di vino con, in quest'ordine, nome, numero di bottiglie
disponibili, annata, costo, ecc. (Si noti che tipicamente un vino viene
prodotto in più annate e in un'annata vengono prodotti molti vini diversi.)

Al termine di ogni richiesta, il Client deve mettersi in attesa della richiesta
successiva, e terminare qualora l'utente inserisca la stringa "fine".

Si realizzino il Server in Unix/C e il Client sia in Unix/C che in Java
*/


#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "utils.h"

/* L'applicazione deve avere la seguente interfaccia: */
/* 	client hostname   porta*/
/* 	argv[0] argv[1]  argv[2] */
int main(int argc, char **argv)
{
	int sd, err, nread;
	uint8_t buffer[4096];
	char insUtente[4096];
	struct addrinfo hints, *res, *ptr;

	if (argc < 3)
	{
		fputs("Errore argomenti!", stderr);
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
	{ // FALLBACK
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
		fputs("Errore connessione!", stderr);
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(res);

	while (1)
	{
        /** VINO */
		printf("Inserisci stringa da mandare DI VINO, 'fine' per terminare\n");
		scanf("%s", &insUtente);

		/* MANDO e scrivo string in formato UTF-8 */

		err = write_all(sd, insUtente, strlen(insUtente));
		if (err < 0)
		{
			fputs("Errore write!", stderr);
			exit(EXIT_FAILURE);
		}

		err = write(sd, "\n", 1);
		if (err < 0)
		{
			fputs("Errore write!", stderr);
			exit(EXIT_FAILURE);
		}

		if (strcmp("fine", insUtente) == 0)
			return 1; // uscita positiva
        
        /**ANNATA */
        printf("Inserisci stringa da mandare DI ANNATA, 'fine' per terminare\n");
		scanf("%s", &insUtente);

		/* MANDO e scrivo string in formato UTF-8 */
		err = write_all(sd, insUtente, strlen(insUtente));
		if (err < 0)
		{
			fputs("Errore write!", stderr);
			exit(EXIT_FAILURE);
		}
		err = write(sd, "\n", 1);
		if (err < 0)
		{
			fputs("Errore write!", stderr);
			exit(EXIT_FAILURE);
		}

		if (strcmp("fine", insUtente) == 0)
			return 1; // uscita positiva


		// Ricezzione risposta e stampa dal server
		nread = read(sd, buffer, sizeof(buffer));
		err = write_all(STDOUT_FILENO, buffer, nread);

		if (err < 0)
		{
			fputs("Errore write!", stderr);
			exit(EXIT_FAILURE);
		}
		if (nread < 0)
		{
			fputs("Errore read!", stderr);
			exit(EXIT_FAILURE);
		}
	}

	close(sd);

	return 0;
}
