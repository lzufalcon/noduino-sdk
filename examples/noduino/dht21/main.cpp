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
#include "dht21.h"

DHT21 dht21(D1);

void ICACHE_FLASH_ATTR
setup()
{
	serial_begin(115200);
}

void ICACHE_FLASH_ATTR
loop()
{
    char t_buf[8];
    char h_buf[8];

	if(dht21.read() == -1) {
		serial_printf("Read sensor error\n");
	} else {
		serial_printf("Temp: %sC, Humi: %s%\n",
			dtostrf(dht21.temperature(), 5, 2, t_buf),
			dtostrf(dht21.humidity(), 5, 2, h_buf));
	}
	delay(2000);
}
