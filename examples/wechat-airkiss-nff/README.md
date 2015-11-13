Noduino SDK - AirKiss NFF
===========================================

This project shows how to use the airkiss nff


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

Using the cutecom to check the Serial port message in Linux:

```bash
$ sudo apt-get install cutecom
```

Run the cutecom, set:

 * Device: /dev/ttyUSB0
 * Baud rate: 115200
 * Data bits: 8
 * Stop bits: 1
 * Parity: None


### Clean

```bash
$ make clean
```

### Document

* http://wiki.jackslab.org/ESP8266_AirKiss_NFF


