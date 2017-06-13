#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <jansson.h>
#include <curl/curl.h>
#include <GeoIP.h>
#include <GeoIPCity.h>

#include "mtmp.h"

int die(char *err, int ret) {

	if(err[0]) fprintf(stderr, "Error: %s\n", err);
	exit(ret);
}

static size_t wresp(void *ptr, size_t size, size_t nmemb, void *stream) {

	wresult *result = (wresult*)stream;

	if(result->pos + size * nmemb >= BUFSZ - 1)
		die("Buffer too small", 1);

	memcpy(result->data + result->pos, ptr, size * nmemb);
	result->pos += size * nmemb;

	return size * nmemb;
}

char *creq(const char *url) {

	CURL *curl = NULL;
	CURLcode status;
	struct curl_slist *headers = NULL;
	char *data = calloc(BUFSZ, sizeof(char));
	long code;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(!curl) die(E_CURL, 1);

	wresult wresult = {data, 0};
	headers = curl_slist_append(headers, "User-Agent: mtmp");

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, wresp);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wresult);

	status = curl_easy_perform(curl);
	if(status != 0) die(E_DATA, 2);

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
	if(code != 200) die(E_HTTP, code);

	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	curl_global_cleanup();

	data[wresult.pos] = 0;

	return data;
}

static weather *geoloc(weather *wtr, const char *ip) {

	GeoIP *gi = GeoIP_open(DB, GEOIP_INDEX_CACHE);
	GeoIPRecord *grec = GeoIP_record_by_name(gi, ip);

	if(grec) {
		strncpy(wtr->loc, grec->city, LOCLEN);
		strncpy(wtr->cc, grec->country_code, CCLEN);
	}

	return wtr;
}

static char *getwdir(int n, char *wdir) {

	if(n > 11 && n < 34) strncpy(wdir, "NNE", WDIRLEN);
	else if(n < 56) strncpy(wdir, "NE", WDIRLEN);
	else if(n < 79) strncpy(wdir, "ENE", WDIRLEN);
	else if(n < 101) strncpy(wdir, "E", WDIRLEN);
	else if(n < 124) strncpy(wdir, "ESE", WDIRLEN);
	else if(n < 146) strncpy(wdir, "SE", WDIRLEN);
	else if(n < 169) strncpy(wdir, "SSE", WDIRLEN);
	else if(n < 191) strncpy(wdir, "S", WDIRLEN);
	else if(n < 214) strncpy(wdir, "SSW", WDIRLEN);
	else if(n < 236) strncpy(wdir, "SW", WDIRLEN);
	else if(n < 259) strncpy(wdir, "WSW", WDIRLEN);
	else if(n < 281) strncpy(wdir, "W", WDIRLEN);
	else if(n < 304) strncpy(wdir, "WNW", WDIRLEN);
	else if(n < 326) strncpy(wdir, "NW", WDIRLEN);
	else if(n < 349) strncpy(wdir, "NNW", WDIRLEN);
	else strncpy(wdir, "N", WDIRLEN);

	return wdir;
}

static void chkobj(json_t *obj) {
	if(!json_is_object(obj)) die(E_FMT, 4);
}

static void chkarr(json_t *obj) {
	if(!json_is_array(obj)) die(E_FMT, 4);
}

static int mainobj(json_t *obj, weather *wtr) {

	chkobj(obj);

	const char *key;
	json_t *tmp;

	json_object_foreach(obj, key, tmp) {
		if(!strncmp(key, "temp", VALLEN))
			wtr->temp = json_real_value(tmp) - 273.15;
		if(!strncmp(key, "humidity", VALLEN))
			wtr->hum = json_integer_value(tmp);
		if(!strncmp(key, "pressure", VALLEN))
			wtr->pres = json_integer_value(tmp);
	}

	if(wtr->temp) return 0;
	else return 1;
}

static int windobj(json_t *obj, weather *wtr) {

	chkobj(obj);

	const char *key;
	json_t *tmp;

	json_object_foreach(obj, key, tmp) {
		if(!strncmp(key, "speed", VALLEN))
			wtr->ws = json_real_value(tmp);
		if(!strncmp(key, "deg", VALLEN))
			getwdir(json_integer_value(tmp), wtr->wdir);
	}

	if(wtr->ws && wtr->wdir[0]) return 0;
	else return 1;
}

static int weatherobj(json_t *obj, weather *wtr) {

	chkarr(obj);
	wtr->desc[0] = 0;

	size_t index;
	const char *key;
	json_t *t1, *t2;

	json_array_foreach(obj, index, t1) {
		chkobj(t1);
		json_object_foreach(t1, key, t2) {
			if(!strncmp(key, "main", VALLEN))
				strncpy(wtr->desc, json_string_value(t2), DESCLEN);
		}
	}

	if(wtr->desc[0]) return 0;
	else return 1;
}

static int sysobj(json_t *obj, weather *wtr) {

	chkobj(obj);

	const char *key;
	json_t *tmp;

	json_object_foreach(obj, key, tmp) {
		if(!strncmp(key, "country", VALLEN))
			strcpy(wtr->cc, json_string_value(tmp));
	}

	if(wtr->cc[0]) return 0;
	else return 1;
}

weather *mtmp(const char *loc, const char *ip, weather *wtr) {

	char url[URLLEN];
	const char *key;
	json_t *root, *obj;

	if(loc[0]) strncpy(wtr->loc, loc, LOCLEN);
	else if(!ip || !ip[0]) die(E_LOC, 8);
	else geoloc(wtr, ip);

	if(!wtr->loc[0]) die(E_GEO, 1);

	snprintf(url, URLLEN, "%s%s&appid=%s", APIURL, wtr->loc, APIKEY);
	char *raw = creq(url);
	if(!raw) die(E_DATA, 2);

	root = json_loads(raw, 0, NULL);
	free(raw);
	if(!root) die(E_JSON, 3);

	json_object_foreach(root, key, obj) {

		if(!strcmp(key, "main")) {
			if(mainobj(obj, wtr)) die(E_READ, 3);

		} else if(!strcmp(key, "wind")) {
			if(windobj(obj, wtr)) die(E_READ, 3);

		} else if(!strcmp(key, "weather")) {
			if(weatherobj(obj, wtr)) die(E_READ, 3);

		} else if(!strcmp(key, "sys")) {
			if(sysobj(obj, wtr)) die(E_READ, 3);

		} else if(!strcmp(key, "name")) {
			strncpy(wtr->loc, json_string_value(obj), LOCLEN);
		}

	}

	return wtr;
}
