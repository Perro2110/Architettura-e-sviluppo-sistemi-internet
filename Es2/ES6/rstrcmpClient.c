#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "utils.h"
#include "rxb.h"

/* L'applicazione deve avere la seguente interfaccia: */
/* 	client  hostname porta   stringa1 stringa2 */
/* 	argv[0] argv[1]  argv[2] argv[3]  argv[4]*/
int main(int argc, char **argv)
{
	int sd, err, nread;
	uint8_t buffer[4096];
	struct addrinfo hints, *res, *ptr;
	rxb_t rxb;
	char risposta[4096];
	size_t risposta_len;

	if (argc != 5)
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

	/* MANDO e scrivo nomefile in formato UTF-8 */
	err = write_all(sd, argv[3], strlen(argv[3]));
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

	err = write_all(sd, argv[4], strlen(argv[4]));
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

	// mRicezzione risposta dal server

	memset(risposta, 0, sizeof(risposta));
	risposta_len = sizeof(risposta) - 1;

    // Inizializza il buffer rxb una sola volta
    rxb_init(&rxb, 64 * 1024);

	err = rxb_readline(&rxb, sd, risposta, &risposta_len);
	if (err < 0)
	{
		fputs("Errore lettura risposta!\n", stderr);
		exit(EXIT_FAILURE);
	}

	// Stampa la risposta
	printf("Risposta: %s\n", risposta);

	// Cleanup (questo codice non verrà mai raggiunto nel programma attuale)
	rxb_destroy(&rxb);

	close(sd);

	return 0;
}
