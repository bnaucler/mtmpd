/*
 *
 *		mtmp - minimal weather service
 *
 */

#ifndef MTMP_HEAD
#define MTMP_HEAD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <jansson.h>
#include <curl/curl.h>
#include <GeoIP.h>
#include <GeoIPCity.h>

#define BUFSZ  8192
#define WBUFSZ 256
#define IPBUFSZ  20
#define LOCLEN  128
#define URLLEN  256
#define VALLEN  128
#define WDIRLEN  4
#define CCLEN 4

#define IPECHO "ipecho.net/plain"
#define APIURL "api.openweathermap.org/data/2.5/weather?q="
#define APIKEY "e3153384df37ea0a3a3d51cc5a08d72d"

#define DB "/var/db/GeoLiteCity.dat"

#define O_NOUINF 0
#define O_UINF 1

typedef struct weather {
	char loc[LOCLEN];
	char cc[CCLEN];
	double temp;
	double ws;
	int wdir;
	int hum;
} weather;

typedef struct wresult {
    char *data;
    int pos;
} wresult;

// Forward declarations - mtmp.c
extern int die(char *err, int uinf, int ret);
extern char *creq(const char *url);
extern char *mtmp(const char *loc, const char *ip, char *ret, const size_t rlen);

#endif
