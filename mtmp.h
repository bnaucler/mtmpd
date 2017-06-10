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

#define BUFSZ  8192
#define LOCLEN  128
#define URLLEN  256
#define VALLEN  128
#define WDIRLEN  4
#define CCLEN 3

#define LAPIURL "ip-api.com/json"
#define WAPIURL "api.openweathermap.org/data/2.5/weather?q="
#define WAPIKEY "e3153384df37ea0a3a3d51cc5a08d72d"

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
extern char *mtmp(const char *loc, const char *ip, char *ret, const size_t rlen);

#endif
