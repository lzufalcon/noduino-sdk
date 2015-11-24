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

void  setup()
{
	uart_init(115200, 115200);
}

void loop()
{
	char buf[128];
	char t[8];
	char h[8];
	memset(t, 0, 8);
	memset(h, 0, 8);

	switch (dht11_read(D1)) {
		case DHT11_OK:
			dtostrf(dht11_temperature(), 5, 2, t);
			dtostrf(dht11_humidity(), 5, 2, h);
			sprintf(buf, "Temp: %sC, Humi: %s%\n", t, h);
			uart0_sendStr(buf);
			break;
		case DHT11_ERROR_CHECKSUM:
			uart0_sendStr("Checksum error\n");
			break;
		case DHT11_ERROR_TIMEOUT:
			uart0_sendStr("Timeout error\n");
			break;
		default:
			uart0_sendStr("Unknown error\n");
			break;
	}

	delay(3000);
}
