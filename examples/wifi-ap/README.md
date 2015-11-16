Noduino SDK - Hello World
===========================================

This simple project shows how to create a wifi ap


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


### Check

Use the iPhone or Laptop to connect the WiFi AP name 'Noduino'


### Clean

```bash
$ make clean
```
