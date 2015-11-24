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

#include "dht21.h"

DHT21::DHT21(int p)
{
	_pin = p;
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, HIGH);
}

void DHT21::set_pin(int p)
{
	_pin = p;
}

char DHT21::read()
{
	char flag;
	pinMode(_pin, OUTPUT);

	/* Bus Low, delay 1 - 5ms */
	digitalWrite(_pin, LOW);
	delay(4);

	/* Bus High, delay 40us */
	digitalWrite(_pin, HIGH);
	delayMicroseconds(40);

	pinMode(_pin, INPUT);
	if (digitalRead(_pin) == 0) {
		flag = 2;
		/* waitting the ACK signal (Low, 80us) */
		while ((digitalRead(_pin) == 0) && flag++) ;
		flag = 2;
		/* waitting the DATA Start signal (High, 80us) */
		while ((digitalRead(_pin) == 1) && flag++) ;
		RH_H = read_8bits();
		RH_L = read_8bits();
		T_H = read_8bits();
		T_L = read_8bits();
		crc = read_8bits();

		pinMode(_pin, OUTPUT);
		digitalWrite(_pin, HIGH);
		return 0;
	} else
		return -1;
}

char DHT21::read_8bits()
{
	char i, flag, data = 0;
	char temp;
	for (i = 0; i < 8; i++) {
		flag = 2;
		while ((digitalRead(_pin) == 0) && flag++) ;
		delayMicroseconds(30);
		temp = 0;
		if (digitalRead(_pin) == 1)
			temp = 1;
		flag = 2;
		while ((digitalRead(_pin) == 1) && flag++) ;
		if (flag == 1)
			break;
		data <<= 1;
		data |= temp;
	}
	return data;
}

char DHT21::data_check()
{
	char tmp = (T_H + T_L + RH_H + RH_L);
	if (tmp != crc) {
		RH_H = 0;
		RH_L = 0;
		T_H = 0;
		T_L = 0;
		return -1;
	} else
		return 0;
}

float DHT21::temperature()
{
	uint16_t T = (T_H << 8) | T_L;
	float tt;

	if (T >= 0)
		tt = T / 10 + (T % 10) * 0.1;
	else {
		T = T & 0x7fff;
		tt = -(T / 10 + (T % 10) * 0.1);
	}

	T_H = 0;
	T_L = 0;

	return tt;
}

float DHT21::humidity()
{
	uint16_t RH = (uint16_t) (RH_H << 8) | RH_L;
	float hum;
	hum = RH / 10 + (RH % 10) * 0.1;

	RH_H = 0;
	RH_L = 0;

	return hum;
}
