#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "utils.h"

/* L'applicazione deve avere la seguente interfaccia: */
/* 	rstrlen hostname porta   */
/* 	argv[0] argv[1]  argv[2] */
int main(int argc, char **argv)
{
	int sd, err, nread;
	uint8_t buffer[4096];
	struct addrinfo hints, *res, *ptr;
	char insUtente[4096];
	
	if(argc != 3) {
		fputs("Errore argomenti!", stderr);
		exit(EXIT_FAILURE);
	}
		


		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
						//hostname  port
		err = getaddrinfo(argv[1], argv[2], &hints, &res);
		if (err != 0) {
			fprintf(stderr, "Errore gai: %s\n", gai_strerror(err));
			exit(EXIT_FAILURE);
		}

		for (ptr = res; ptr != NULL; ptr = ptr->ai_next) { // FALLBACK
			sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (sd < 0) continue;

			err = connect(sd, ptr->ai_addr, ptr->ai_addrlen);
			if (err == 0) break;

			close(sd);
		}

		if (ptr == NULL) {
			fputs("Errore connessione!", stderr);
			exit(EXIT_FAILURE);
		}

		freeaddrinfo(res);



	while(1){
		printf("Inserisci stringa da valutare, 'fine' per terminare\n");
		scanf("%s",&insUtente);

		/* MANDO e scrivo string in formato UTF-8 */

		err = write_all(sd, insUtente, strlen(insUtente));
		if (err < 0) {
			fputs("Errore write!", stderr);
			exit(EXIT_FAILURE);
		}

		err = write(sd, "\n", 1);
		if (err < 0) {
			fputs("Errore write!", stderr);
			exit(EXIT_FAILURE);
		}
		
		if (strcmp("fine",insUtente) == 0)
			return 1; //uscita positiva

		//mRicezzione risposta e stampa dal server 
		while ((nread = read(sd, buffer, sizeof(buffer))) > 0) {
			err = write_all(STDOUT_FILENO, buffer, nread);
			if (err < 0) {
				fputs("Errore write!", stderr);
				exit(EXIT_FAILURE);
			}
		}
		if (nread < 0) {
			fputs("Errore read!", stderr);
			exit(EXIT_FAILURE);
		}
	}

	close(sd);
	return 0;
}

