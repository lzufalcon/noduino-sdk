/*
 *  Copyright (c) 2015 - 2025 MaiKe Labs
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
#include "noduino.h"
#include "dht11.h"

void setup()
{
	serial_begin(115200);
}

void loop()
{
	char t_buf[8];
	char h_buf[8];

	switch (dht11_read(D1)) {
		case DHT11_OK:
			serial_printf("Temp: %sC, Humi: %s%\n",
				dtostrf(dht11_temperature(), 5, 2, t_buf),
				dtostrf(dht11_humidity(), 5, 2, h_buf));
			break;
		case DHT11_ERROR_CHECKSUM:
			serial_printf("Checksum error\n");
			break;
		case DHT11_ERROR_TIMEOUT:
			serial_printf("Timeout error\n");
			break;
		default:
			serial_printf("Unknown error\n");
			break;
	}

	delay(3000);
}
