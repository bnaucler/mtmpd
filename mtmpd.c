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

#define ERR_RESP "Sorry! Could not retrieve weather data.\n"

int sfd;

int matoi(char* str) {

    long lret = strtol(str, NULL, 10);

    if(lret > INT_MAX) lret = INT_MAX;
    else if(lret < INT_MIN) lret = INT_MIN;

    return (int)lret;
}

static void sighandler(int sig) {

	syslog(LOG_DAEMON | LOG_NOTICE, "Received SIGTERM - shutting down.");
	close(sfd);
	exit(1);
}

static void disco(const int sock, const char *err) {

	syslog(LOG_DAEMON | LOG_ERR, err);
	write(sock, ERR_RESP, strlen(ERR_RESP));
	shutdown(sock, 1);
}

static char *addnewline(char *str) {

	int len = strlen(str);

	str[len++] = '\n';
	str[len] = 0;

	return str;
}

static void acceptclient(const int sock, struct sockaddr_in *client,
	socklen_t *clilen) {

	int rc;
	char cip[IPBUFSZ];
	char wstr[WSTRLEN];
	weather wtr;

	rc = getnameinfo((struct sockaddr*)client, *clilen, cip,
		IPBUFSZ, 0 , 0, NI_NUMERICHOST);
	if(rc) return disco(sock, "Failed IP address conversion");

	mtmp("", cip, &wtr);
	if(!wtr.temp) return disco(sock, "mtmp failed");
	mkwstr(&wtr, wstr, sizeof(wstr));
	addnewline(wstr);

	syslog(LOG_DAEMON | LOG_INFO, "Client %s: %s", cip, wstr);
	rc = write(sock, wstr, strlen(wstr));

	if(rc < 0) syslog(LOG_DAEMON | LOG_ERR, "Could not write to socket");

	shutdown(sock, 1);
}

int main(int argc, char *argv[]) {

	int csfd, pid;
	socklen_t clilen;

	struct sockaddr_in server, client;
	memset(&server, 0, sizeof(server));

	if(argc < 2) die("No port number specified", 1);
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd < 0) die("Could not open socket", 2);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(matoi(argv[1]));

	if(bind(sfd, (struct sockaddr*)&server, sizeof(server)))
		die("Could not bind port to socket", 3);

	listen(sfd, MXCLI);
	clilen = sizeof(client);

	daemon(0, 0);

	signal(SIGCHLD, SIG_IGN);
	signal(SIGTERM, sighandler);

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

	return 0;
}
