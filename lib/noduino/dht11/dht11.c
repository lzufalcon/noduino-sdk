/*
 *  Copyright (c) 2015 - 2025 MaiKe Labs
 *  Library for DHT11 digital temperature sensor
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

#include "dht11.h"

float ICACHE_FLASH_ATTR dht11_humidity()
{
	return (float)dht11_humi;
}

float ICACHE_FLASH_ATTR dht11_temperature()
{
	return (float)dht11_temp;
}

int ICACHE_FLASH_ATTR dht11_read(int pin)
{
	// BUFFER TO RECEIVE
	uint8_t bits[5];
	uint8_t cnt = 7;
	uint8_t idx = 0;

	// EMPTY BUFFER
	for (int i = 0; i < 5; i++)
		bits[i] = 0;

	// REQUEST SAMPLE
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
	delay(18);
	digitalWrite(pin, HIGH);
	delayMicroseconds(40);
	pinMode(pin, INPUT);

	// ACKNOWLEDGE or TIMEOUT
	unsigned int loopCnt = 10000;
	while (digitalRead(pin) == LOW)
		if (loopCnt-- == 0)
			return DHT11_ERROR_TIMEOUT;

	loopCnt = 10000;
	while (digitalRead(pin) == HIGH)
		if (loopCnt-- == 0)
			return DHT11_ERROR_TIMEOUT;

	// READ OUTPUT - 40 BITS => 5 BYTES or TIMEOUT
	for (int i = 0; i < 40; i++) {
		loopCnt = 10000;
		while (digitalRead(pin) == LOW)
			if (loopCnt-- == 0)
				return DHT11_ERROR_TIMEOUT;

		unsigned long t = micros();

		loopCnt = 10000;
		while (digitalRead(pin) == HIGH)
			if (loopCnt-- == 0)
				return DHT11_ERROR_TIMEOUT;

		if ((micros() - t) > 40)
			bits[idx] |= (1 << cnt);
		if (cnt == 0)	// next byte?
		{
			cnt = 7;	// restart at MSB
			idx++;	// next byte!
		} else
			cnt--;
	}

	// WRITE TO RIGHT VARS
	// as bits[1] and bits[3] are allways zero they are omitted in formulas.
	dht11_humi = bits[0];
	dht11_temp = bits[2];

	uint8_t sum = bits[0] + bits[2];

	if (bits[4] != sum)
		return DHT11_ERROR_CHECKSUM;
	return DHT11_OK;
}
