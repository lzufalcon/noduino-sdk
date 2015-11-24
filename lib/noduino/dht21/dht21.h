/*
 *  Copyright (c) 2015 - 2025 MaiKe Labs
 *  Library for DHT21/AM2301 digital temperature sensor
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
*/

#ifndef __DHT21_H__
#define __DHT21_H__

#include "noduino.h"

class DHT21 {
 public:
	DHT21(int pin);
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
