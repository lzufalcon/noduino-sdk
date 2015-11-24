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

#ifndef __DHT11_H__
#define __DHT11_H__

#include "noduino.h"

#define	DHT11_OK				 0
#define	DHT11_ERROR_CHECKSUM	-1
#define	DHT11_ERROR_TIMEOUT		-2

int dht11_read(int pin);
float dht11_humidity();
float dht11_temperature();
int dht11_humi;
int dht11_temp;

#endif
