#include <stdint.h>
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "rxb.h"

// server  porta
// argv[0] argv[1]
int main(int argc, char *argv[])
{
	int sd, err;
	char *service;
	struct addrinfo hints, *res;

	/* Controllo argomenti */
	if (argc != 2) {
		fprintf(stderr, "Usage:\n\t%s port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Per ripristinare un comportamento sensato delle socket */
	signal(SIGPIPE, SIG_IGN);

	service = argv[1];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; /* IPv4 o IPv6 */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; //IN SERVER PERCHE NON CHIEDIAMO LA RISOLUZIONE DI UN NOME MA LA PREPARAZIONE DELLA STRUTTURA DA PASSARE A BIND

	err = getaddrinfo(NULL, service, &hints, &res);
	if (err != 0) {
		fprintf(stderr, "Error: %s\n", gai_strerror(err));
		exit(EXIT_FAILURE);
	}

	sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	err = bind(sd, res->ai_addr, res->ai_addrlen);
	if (err < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	err = listen(sd, SOMAXCONN);
	if (err < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	while (1) {
		rxb_t rxb;
		int ns;
		char command_opt[1024];
		size_t command_opt_len;

		ns = accept(sd, NULL, NULL); //SOKET
		if (ns < 0) {
			if (errno == EINTR)
				continue;
			perror("accept");
			exit(EXIT_FAILURE);
		}

		/* Alloco la memoria per il buffer rxb */
		rxb_init(&rxb, 64 * 1024);

		/* Inizializzo a zero il buffer per il command_opt */
		memset(command_opt, 0, sizeof(command_opt));
		command_opt_len = sizeof(command_opt)-1;

		/* Chiamo rxb_readline */
		err = rxb_readline(&rxb, ns, command_opt, &command_opt_len);
		if (err < 0) {
			perror("rxb_readline");
			exit(EXIT_FAILURE);
		}

#ifdef USE_LIBUNISTRING
		/* Controllo validità UTF-8 */
		if (u8_check((uint8_t *)command_opt, command_opt_len) != NULL) {
			fprintf(stderr, "Invalid input\n");
			exit(EXIT_FAILURE);
		}
#endif

        if(strcmp("null",command_opt) != 0){
           
            int pid, status;

            pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                /* Redirigo output su socket */
                close(1);
                dup(ns);
                close(ns);
                
                /* Chiamo exec head */
                execlp("ps", "ps", command_opt, NULL);

                /* Se sono arrivato qui, è accaduto qualcosa di orribile... */
                perror("execlp");
                exit(EXIT_FAILURE);
            }
            
            wait(&status);
            rxb_destroy(&rxb);
            close(ns);

        }else{ // mi è arrivato null

            int pid, status;

            pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                /* Redirigo output su socket */
                close(1);
                dup(ns);
                close(ns);
                
                /* Chiamo exec head */
                execlp("ps", "ps", NULL);

                /* Se sono arrivato qui, è accaduto qualcosa di orribile... */
                perror("execlp");
                exit(EXIT_FAILURE);
            }
            
            wait(&status);
            rxb_destroy(&rxb);
            close(ns);

        }
	}

	close(sd);

	return 0;
}
