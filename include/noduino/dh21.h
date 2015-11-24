/*
 * dh21.h - Library for DH21/AM2301 digital temperature sensor
 * Created by Jack Tan <jiankemeng@gmail.com>
 * Released into the public domain.
 */
#ifndef _DH21_H_
#define _DH21_H_

#include "noduino.h"

class DH21 {
 public:
	DH21(int pin);
	float temperature();
	float humidity();
	char read();
	char data_check();
	void set_pin(int p);

 private:
	char read_8bits();
	int _pin;
	char RH_H;
	char RH_L;
	char T_H;
	char T_L;
	char crc;
};
#endif
