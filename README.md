# mtmpd 0.1
A minimal socket implementation if mtmp

## Written by
Björn Westerberg Nauclér (mail@bnaucler.se) 2017

Tested on Arch Linux 4.11 (x86\_64)

## Dependencies
mtmp requires libjansson and libcurl to compile, and connects to [OpenWeathermap](http://openweathermap.org) and [ip-api](http://ip-api.com) for data retrieval.

## Installation
`sudo make all install`

## Usage
`mtmpd <port>` starts the service at port \<port\>

## Disclaimer
Wrote this to experiment with libcurl and libjansson; not to create a solid product.

Currently my personal (limited) API key to OpenWeathermap is hardcoded in the source. Please change to your own key if you intend to send more than an occasional request.

## License
MIT (do whatever you want)
