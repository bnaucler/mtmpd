#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <jansson.h>
#include <curl/curl.h>

#include "mtmp.h"

int die(char *err, int uinf, int ret) {

	if(err[0]) fprintf(stderr, "Error: %s\n", err);
	exit(ret);
}

static size_t wresp(void *ptr, size_t size, size_t nmemb, void *stream) {

    wresult *result = (wresult*)stream;

    if(result->pos + size * nmemb >= BUFSZ - 1)
		die("Buffer too small", O_NOUINF, 1);

    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}

static char *creq(const char *url) {

    CURL *curl = NULL;
    CURLcode status;
    struct curl_slist *headers = NULL;
    char *data = calloc(BUFSZ, sizeof(char));
    long code;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(!curl) die("Could not start curl", O_NOUINF, 1);

    wresult wresult = {data, 0};
    headers = curl_slist_append(headers, "User-Agent: mtmp");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, wresp);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wresult);

    status = curl_easy_perform(curl);
    if(status != 0) die("Could not read data", O_NOUINF, 2);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if(code != 200) die("Returning error code from server", O_NOUINF, code);

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();

    data[wresult.pos] = 0;

    return data;
}

static char *geoloc(const char *ip, char *ret, size_t len) {

	char req[URLLEN];
	char *raw;

	if(!ip[1]) strncpy(req, LAPIURL, URLLEN);
	else snprintf(req, URLLEN, "%s/%s", LAPIURL, ip);
	raw = creq(req);

	json_t *root;
	json_error_t err;

	root = (json_loads(raw, 0, &err));
	free(raw);

	if(!json_is_object(root)) return NULL;

	const char *key;
	json_t *val;
	json_object_foreach(root, key, val) { if(!strncmp(key, "city", len)) break; }
	strncpy(ret, json_string_value(val), len);

	return ret;
}

static char *getwdir(int wdir, char *twdir) {

	if(wdir > 11 && wdir < 34) strncpy(twdir, "NNE", WDIRLEN);
	else if(wdir < 56) strncpy(twdir, "NE", WDIRLEN);
	else if(wdir < 79) strncpy(twdir, "ENE", WDIRLEN);
	else if(wdir < 101) strncpy(twdir, "E", WDIRLEN);
	else if(wdir < 124) strncpy(twdir, "ESE", WDIRLEN);
	else if(wdir < 146) strncpy(twdir, "SE", WDIRLEN);
	else if(wdir < 169) strncpy(twdir, "SSE", WDIRLEN);
	else if(wdir < 191) strncpy(twdir, "S", WDIRLEN);
	else if(wdir < 214) strncpy(twdir, "SSW", WDIRLEN);
	else if(wdir < 236) strncpy(twdir, "SW", WDIRLEN);
	else if(wdir < 259) strncpy(twdir, "WSW", WDIRLEN);
	else if(wdir < 281) strncpy(twdir, "W", WDIRLEN);
	else if(wdir < 304) strncpy(twdir, "WNW", WDIRLEN);
	else if(wdir < 326) strncpy(twdir, "NW", WDIRLEN);
	else if(wdir < 349) strncpy(twdir, "NNW", WDIRLEN);
	else strncpy(twdir, "N", WDIRLEN);

	return twdir;
}

static void capfirst(char *str) {

	int scap = 0;
	char *sptr = str;

	do  {
		if(!scap++) *sptr = toupper(*sptr);
		else if (isspace(*sptr)) scap = 0;
	} while(*++sptr);
}

char *mtmp(const char *loc, const char *ip, char *ret, const size_t rlen) {

	char url[URLLEN];
	char twdir[4];
	weather wtr;

	json_t *root;
	json_error_t err;

	if(loc[0]) strncpy(wtr.loc, loc, LOCLEN);
	else if(!ip[0]) die("Could not determine location", O_NOUINF, 8);
	else strncpy(wtr.loc, geoloc(ip, wtr.loc, LOCLEN), LOCLEN);

	if(!wtr.loc[0]) die("Could not retrieve geolocation", O_NOUINF, 1);
	capfirst(wtr.loc);

	snprintf(url, URLLEN, "%s%s&appid=%s", WAPIURL, wtr.loc, WAPIKEY);
	char *raw = creq(url);
	if(!raw) die("Could not read data", O_NOUINF, 2);

	root = (json_loads(raw, 0, &err));
	free(raw);
	if(!root) die("JSON decoding failed", O_NOUINF, 3);

	const char *k1, *k2;
	json_t *v1, *v2;

	json_object_foreach(root, k1, v1) {
		if(!strcmp(k1, "main")) {
			if(!json_is_object(v1)) die("Unexpected JSON format", O_NOUINF, 4);
			json_object_foreach(v1, k2, v2) {
				if(!strncmp(k2, "temp", VALLEN)) wtr.temp = json_real_value(v2) - 273.15;
				if(!strncmp(k2, "humidity", VALLEN)) wtr.hum = json_integer_value(v2);
			}

		} else if(!strcmp(k1, "wind")) {
			if(!json_is_object(v1)) die("Unexpected JSON format", O_NOUINF, 4);
			json_object_foreach(v1, k2, v2) {
				if(!strncmp(k2, "speed", VALLEN)) wtr.ws = json_real_value(v2);
				if(!strncmp(k2, "deg", VALLEN)) wtr.wdir = json_integer_value(v2);
			}

		} else if(!strcmp(k1, "sys")) {
			if(!json_is_object(v1)) die("Unexpected JSON format", O_NOUINF, 4);
			json_object_foreach(v1, k2, v2) {
				if(!strncmp(k2, "country", VALLEN))
					strcpy(wtr.cc, json_string_value(v2));
			}

		}
	}

	snprintf(ret, rlen, "%s, %s: %.1fC, humidity: %d%%, %.1f m/s %s\n",
			wtr.loc, wtr.cc, wtr.temp, wtr.hum, wtr.ws, getwdir(wtr.wdir, twdir));

	return ret;
}
