/*
 *  Copyright (c) 2015 - 2025 MaiKe Labs
 *
 *  Connect VCC of the TSL2561 sensor to 3.3V
 *  Connect GND to Ground
 *  Connect SCL to i2c clock - GPIO4
 *  Connect SDA to i2c data  - GPIO5
 *
 *  ADDR can be connected to ground, or VCC or left floating to change
 *  the i2c address. The address will be different depending on whether
 *  you let. the ADDR pin float (addr 0x39), or tie it to ground or vcc.
 *  In those cases use TSL2561_ADDR_LOW (0x29) or TSL2561_ADDR_HIGH (0x49)
 *  respectively TSL2561 tsl(TSL2561_ADDR_FLOAT) 
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
#include "tsl2561.h"

irom void setup()
{
	serial_begin(115200);

	if (!tsl2561_begin(TSL2561_ADDR_FLOAT)) {
		serial_print("Could not found TSL2561 sensor!\r\n");

		while (1) {}
	}

	// You can change the gain on the fly, to adapt to brighter/dimmer
	// light situations
	//tsl2561_setGain(TSL2561_GAIN_0X);		// set no gain (for bright situtations)
	tsl2561_setGain(TSL2561_GAIN_16X);		// set 16x gain (for dim situations)

	// Changing the integration time gives you a longer time over which to sense light
	// longer timelines are slower, but are good in very low light situtations!
	tsl2561_setTiming(TSL2561_INTEGRATIONTIME_13MS);		// shortest integration time (bright light)
	//tsl2561_setTiming(TSL2561_INTEGRATIONTIME_101MS);		// medium integration time (medium light)
	//tsl2561_setTiming(TSL2561_INTEGRATIONTIME_402MS);		// longest integration time (dim light)
}
  
irom void loop()
{
	// Simple data read example. Just read the infrared, fullspecrtrum diode 
	// or 'visible' (difference between the two) channels.
	// This can take 13-402 milliseconds! Uncomment whichever of the following you want to read
	uint16_t x = tsl2561_getLuminosity(TSL2561_VISIBLE);     
	//uint16_t x = tsl2561_getLuminosity(TSL2561_FULLSPECTRUM);
	//uint16_t x = tsl2561_getLuminosity(TSL2561_INFRARED);

	serial_printf("Luminosity:\t %d\r\n", x);

	// More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
	// That way you can do whatever math and comparisons you want!
	uint32_t lum = tsl2561_getFullLuminosity();
	uint16_t ir, full;
	ir = lum >> 16;
	full = lum & 0xFFFF;

	serial_printf("IR:\t\t %d\r\n", ir);
	serial_printf("Full: ", full);
	serial_printf("Visible:\t %d\r\n", full - ir);
	serial_printf("Lux:\t\t %d\r\n\n", tsl2561_calculateLux(full, ir));

	delay(3000);
}
