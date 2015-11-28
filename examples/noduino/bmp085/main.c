/*
 *  Copyright (c) 2015 - 2025 MaiKe Labs
 *
 *  Connect VCC of the BMP085 sensor to 3.3V (NOT 5.0V!)
 *  Connect GND to Ground
 *  Connect SCL to i2c clock - GPIO4
 *  Connect SDA to i2c data  - GPIO5
 *  EOC is not used, it signifies an end of conversion
 *  XCLR is a reset pin, also not used here
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

void ICACHE_FLASH_ATTR setup()
{
	serial_begin(9600);
	if (!bmp085_begin()) {
		serial_print("Could not find a valid BMP085 sensor, check wiring!\n");
		while (1) {}
	}
}
  
void ICACHE_FLASH_ATTR loop()
{
	char out_buf[7];

	serial_printf("Temperature = %s C\n", dtostrf(bmp085_readTemperature(), 6, 2, out_buf));
	serial_printf("Pressure = %d\n", bmp085_readPressure());

	// Calculate altitude assuming 'standard' barometric
	// pressure of 1013.25 millibar = 101325 Pascal
	serial_printf("Altitude = %s M\n", dtostrf(bmp085_readAltitude(), 6, 2, out_buf));
	serial_printf("Pressure at sealevel (calculated) = %d Pa\n",
			bmp085_readSealevelPressure());

	// you can get a more precise measurement of altitude
	// if you know the current sea level pressure which will
	// vary with weather and such. If it is 1015 millibars
	// that is equal to 101500 Pascals.
	serial_printf("Real altitude = %d M", bmp085_readAltitude(101500));

	delay(3000);
}
