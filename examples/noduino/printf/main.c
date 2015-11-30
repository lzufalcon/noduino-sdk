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

void setup()
{
	serial_begin(115200);
}

void loop()
{
	int addr = 9;

	serial_print("Hello World!\r\n");		// only print strings

	serial_printf("%d, %s, %p, 0x%X\r\n",
		  			addr, "Hello World!", &addr, 128);

	serial_printf("%f\r\n", 11.2);		// can not support float

	// print float or double:
	char out_buf[7];
	serial_printf("float: |%s|\r\n", dtostrf(13.6672, 6, 3, out_buf));
	serial_printf("float: |%s|\r\n", dtostrf(13.6672, 6, 1, out_buf));
	serial_printf("float: |%s|\r\n", dtostrf(13.6672, 3, 3, out_buf));

	delay(1800);						// delay 1800ms
}
