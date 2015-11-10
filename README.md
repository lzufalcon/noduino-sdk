Noduino SDK
===========================================

This is a Professional SDK developed by MaKe Labs for ESP8266 WiFi SoC

This project brings support for ESP8266 chip to the professional embeded environment. It lets you write sketches using C language quickly, and run them directly on ESP8266, no external microcontroller required.

It has a lot of cool examples and reference sketches. You can make a smart IoT devices quickly based on these sketches.


### Quick Start

- NodeMCU v1.0 board / Noduino Falcon board
- Git

```bash
# clone the whole sdk
$ git clone git://github.com/icamgo/noduino-sdk.git noduino-sdk

# fetch the toolchain of esp8266
$ cd noduino-sdk
$ git submodule init
$ git submodule update
$ cd toolchain
$ ./gen.py
# generate the toolchain (you need Python 2.7)

# try the first example
$ cd ../example/blink
$ make

# upload the example to dev board through serial
$ make flash
```

You can try other cool examples in example directory
