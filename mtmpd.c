#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "mtmp.h"

#define MXCLI 5
#define RESP "Message recieved"

int matoi(char* str) {

    long lret = strtol(str, NULL, 10);

    if(lret > INT_MAX) lret = INT_MAX;
    else if(lret < INT_MIN) lret = INT_MIN;

    return (int)lret;
}

static void acceptclient(int sock, struct sockaddr_in *client, socklen_t *clilen) {

	int rc;
	char cip[IPBUFSZ];
	char wstr[WBUFSZ];

	rc = getnameinfo((struct sockaddr*)client, *clilen, cip,
	sizeof(cip), 0 , 0, NI_NUMERICHOST);
	if(rc) die("Failed to convert address to string", O_NOUINF, 7);

	printf("Connection from: %s\n", cip);
	mtmp("", cip, wstr, sizeof(wstr));

	rc = write(sock, wstr, strlen(wstr));
	if(rc < 0) die("Could not write to socket", O_NOUINF, 6);
}

int main(int argc, char *argv[]) {

	int sfd, csfd, pnum, pid;
	socklen_t clilen;

	struct sockaddr_in server, client;
	memset(&server, 0, sizeof(server));

	if(argc < 2) die("No port number specified", O_NOUINF, 1);
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd < 0) die("Could not open socket", O_NOUINF, 2);

	pnum = matoi(argv[1]);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(pnum);

	if(bind(sfd, (struct sockaddr*)&server, sizeof(server)))
		die("Could not bind port to socket", O_NOUINF, 3);

	listen(sfd, MXCLI);
	clilen = sizeof(client);

	for(;;) {
		csfd = accept(sfd, (struct sockaddr*)&client, &clilen);
		if(csfd < 0) die("Could not connect to client", O_NOUINF, 4);

		pid = fork();

		if(pid < 0) {
			die("Could not fork()", O_NOUINF, 4);

		} else if (!pid) {
			close(sfd);
			acceptclient(csfd, &client, &clilen);
			exit(0);

		} else {
			close(csfd);
		}

		close(csfd);
	}

	close(sfd);

	return 0;
}
