#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <syslog.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "mtmp.h"

#define MXCLI 5

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
	if(rc) syslog(LOG_DAEMON | LOG_ERR, "Failed IP address conversion");

	syslog(LOG_DAEMON | LOG_INFO, "Connection from %s", cip);
	mtmp("", cip, wstr, sizeof(wstr));

	rc = write(sock, wstr, strlen(wstr));
	if(rc < 0) syslog(LOG_DAEMON | LOG_ERR, "Could not write to socket");

	shutdown(sock, 1);
}

int main(int argc, char *argv[]) {

	int sfd, csfd, pid;
	socklen_t clilen;

	struct sockaddr_in server, client;
	memset(&server, 0, sizeof(server));

	if(argc < 2) die("No port number specified", O_NOUINF, 1);
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd < 0) die("Could not open socket", O_NOUINF, 2);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(matoi(argv[1]));

	if(bind(sfd, (struct sockaddr*)&server, sizeof(server)))
		die("Could not bind port to socket", O_NOUINF, 3);

	listen(sfd, MXCLI);
	clilen = sizeof(client);

	daemon(0, 0);
	signal(SIGCHLD, SIG_IGN);

	syslog(LOG_DAEMON | LOG_NOTICE, "Launched into the background");

	for(;;) {
		csfd = accept(sfd, (struct sockaddr*)&client, &clilen);
		if(csfd < 0) syslog(LOG_DAEMON | LOG_ERR, "Could not connect to client");

		pid = fork();

		if(pid < 0) {
			syslog(LOG_DAEMON | LOG_ERR, "Could not fork()");

		} else if (!pid) {
			close(sfd);
			acceptclient(csfd, &client, &clilen);
			exit(0);
		}

		close(csfd);
	}

	close(sfd);

	return 0;
}
