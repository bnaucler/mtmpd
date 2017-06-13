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
#define DESCLEN 64
#define URLLEN  256
#define VALLEN  128
#define WDIRLEN  4
#define CCLEN 4

#define IPECHO "ipecho.net/plain"
#define APIURL "api.openweathermap.org/data/2.5/weather?q="
#define APIKEY "e3153384df37ea0a3a3d51cc5a08d72d"

#define DB "/var/db/GeoLiteCity.dat"

#define E_GEO "Could not retrieve geolocation"
#define E_LOC "Could not determine location"
#define E_DATA "Could not read data"
#define E_JSON "JSON decoding failed"
#define E_FMT "Unexpected JSON format"
#define E_READ "Could not read data"
#define E_CURL "Could not execute curl"
#define E_HTTP "HTTP error code returned from server"

typedef struct weather {
	char loc[LOCLEN];
	char cc[CCLEN];
	char desc[DESCLEN];
	char wdir[WDIRLEN];
	double temp;
	double ws;
	int hum;
	int pres;
} weather;

typedef struct wresult {
    char *data;
    int pos;
} wresult;

// Forward declarations - mtmp.c
extern int die(char *err, int ret);
extern char *creq(const char *url);
extern weather *mtmp(const char *loc, const char *ip, weather *wtr);

#endif
