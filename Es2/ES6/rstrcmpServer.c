// gcc -g -o ./eseS rstrcmpServer.c rxb.c rxb.h utils.c utils.h
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

// server  porta
// argv[0] argv[1]
int main(int argc, char *argv[])
{
	int sd, err;
	int optval = 1;
	char *service;
	struct addrinfo hints, *res;

	/* Controllo argomenti */
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\t%s port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Per ripristinare un comportamento sensato delle socket */
	signal(SIGPIPE, SIG_IGN);

	service = argv[1];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; /* IPv4 o IPv6 */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // IN SERVER PERCHE NON CHIEDIAMO LA RISOLUZIONE DI UN NOME MA LA PREPARAZIONE DELLA STRUTTURA DA PASSARE A BIND

	err = getaddrinfo(NULL, service, &hints, &res);

	if (err != 0)
	{
		fprintf(stderr, "Error: %s\n", gai_strerror(err));
		exit(EXIT_FAILURE);
	}

	sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sd < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	err = setsockopt(sd,
					 SOL_SOCKET,   // Livello dell'opzione
					 SO_REUSEPORT, // Opzione da impostare
					 &optval,	   // Puntatore al valore
					 sizeof(optval));

	if (err < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	err = bind(sd, res->ai_addr, res->ai_addrlen);
	if (err < 0)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	err = listen(sd, SOMAXCONN);
	if (err < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		rxb_t rxb;
		int ns;
		char str1[1024];
		char str2[1024];

		size_t str1_len;
		size_t str2_len;

		ns = accept(sd, NULL, NULL); // SOKET
		if (ns < 0)
		{
			if (errno == EINTR)
				continue;
			perror("accept");
			exit(EXIT_FAILURE);
		}

		int pid = fork();
		if (pid == 0)
		{
			// close(sd);
			// while (1)
			//{
			/* Alloco la memoria per il buffer rxb */
			rxb_init(&rxb, 64 * 1024);

			/* Inizializzo a zero il buffer per il str1 */
			memset(str1, 0, sizeof(str1));
			str1_len = sizeof(str1) - 1;

			/* Inizializzo a zero il buffer per il str2 */
			memset(str2, 0, sizeof(str2));
			str2_len = sizeof(str2) - 1;

			/* Chiamo rxb_readline */
			err = rxb_readline(&rxb, ns, str1, &str1_len);
			if (err < 0)
			{
				perror("rxb_readline");
				exit(EXIT_FAILURE);
			}

#ifdef USE_LIBUNISTRING
			/* Controllo validità UTF-8 */
			if (u8_check((uint8_t *)str1, str1_len) != NULL)
			{
				fprintf(stderr, "Invalid input\n");
				exit(EXIT_FAILURE);
			}
#endif

			/* Chiamo rxb_readline */
			err = rxb_readline(&rxb, ns, str2, &str2_len);
			if (err < 0)
			{
				perror("rxb_readline");
				exit(EXIT_FAILURE);
			}

#ifdef USE_LIBUNISTRING
			/* Controllo validità UTF-8 */
			if (u8_check((uint8_t *)str2, str2_len) != NULL)
			{
				fprintf(stderr, "Invalid input\n");
				exit(EXIT_FAILURE);
			}
#endif
			if (strcmp(str1, str2) == 0)
			{
				err = write_all(ns, "SI", strlen("SI"));
				if (err < 0)
				{
					fputs("Errore write!1", stderr);
					exit(EXIT_FAILURE);
				}

				err = write(ns, "\n", 1);
				if (err < 0)
				{
					fputs("Errore write!2", stderr);
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				err = write_all(ns, "NO", strlen("NO"));
				if (err < 0)
				{
					fputs("Errore write!3", stderr);
					exit(EXIT_FAILURE);
				}

				err = write(ns, "\n", 1);
				if (err < 0)
				{
					fputs("Errore write!4", stderr);
					exit(EXIT_FAILURE);
				}
			}
			rxb_destroy(&rxb);
			//}
			close(ns);
			exit(0);
		}
	}
	close(sd);
	return 0;
}