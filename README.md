# mtmpd 0.1
A minimal http server for weather data built on [mtmp](https://github.com/bnaucler/mtmp).

## Written by
Björn Westerberg Nauclér (mail@bnaucler.se) 2017

Tested on Arch Linux 4.11 (x86\_64)

## Dependencies
mtmpd requires libjansson, libcurl and libgeoip, and connects to [OpenWeathermap](http://openweathermap.org) for data retrieval.

You will also need to download the [GeoLite City database](http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz) from Maxmind, and unzip to `/var/db/GeoLiteCity.dat`

## Installation
`sudo make all install`

## Usage
`mtmpd <port>` starts the service at port \<port\>

mtmpd launches as a deamon and logs messages via syslog.

## Disclaimer
Wrote this to experiment with libcurl and libjansson; not to create a solid product.

Currently my personal (limited) API key to OpenWeathermap is hardcoded in the source. Please change to your own key if you intend to send more than an occasional request.

## License
MIT (do whatever you want)
