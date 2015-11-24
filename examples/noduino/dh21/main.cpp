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
#include "dh21.h"

DH21 dh21(D1);

void ICACHE_FLASH_ATTR
setup()
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
}

void ICACHE_FLASH_ATTR
loop()
{
	char buf[128];
	char t[8];
    char h[8];
	memset(t, 0, 8);
	memset(h, 0, 8);

	if(dh21.read() == -1) {
		uart0_sendStr("Read sensor error\n");
	} else {
		dtostrf(dh21.temperature(), 5, 2, t);
		dtostrf(dh21.humidity(), 5, 2, h);
		sprintf(buf, "Temp: %sC, Humi: %s%\n", t, h);
		uart0_sendStr(buf);
	}
	delay(2000);
}
