Noduino SDK - Blink example
===========================================

This simple project show how to flush the blue LED in NodeMCU V1.0 board
(using ESP-12E / ESP-12F module)
The ESP-12E / ESP-12F use the GPIO2 to control the onboard blue LED.


### Build

```bash
$ make
```

Or make output more message:

```bash
$ make V=1
```

The target file is at build/ directory.


### Upload

```bash
$ make flash
```

It use the ```/dev/ttyUSB0``` serial device to upload the firmware into board.

You need to modify the varible ```ESPPORT``` according to your system in
Makefile. It should be /dev/cu.SLAB_USBtoUART in Mac OS X or COM3 in windows.


### Clean

```bash
$ make clean
```

