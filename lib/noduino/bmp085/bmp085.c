/*
 *  Copyright (c) 2015 - 2025 MaiKe Labs
 *  Library for sensor BMP085
 *  This library is ported from adafruit Arduino BMP085 project
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

#include "bmp085.h"

bool ICACHE_FLASH_ATTR bmp085_begin()
{
  uint8_t mode = BMP085_ULTRAHIGHRES;
  if (mode > BMP085_ULTRAHIGHRES) 
    mode = BMP085_ULTRAHIGHRES;
  bmp085_oversampling = mode;

  wire_begin();

  if (bmp085_read8(0xD0) != 0x55) return false;

  /* bmp085_read calibration data */
  ac1 = bmp085_read16(BMP085_CAL_AC1);
  ac2 = bmp085_read16(BMP085_CAL_AC2);
  ac3 = bmp085_read16(BMP085_CAL_AC3);
  ac4 = bmp085_read16(BMP085_CAL_AC4);
  ac5 = bmp085_read16(BMP085_CAL_AC5);
  ac6 = bmp085_read16(BMP085_CAL_AC6);

  b1 = bmp085_read16(BMP085_CAL_B1);
  b2 = bmp085_read16(BMP085_CAL_B2);

  mb = bmp085_read16(BMP085_CAL_MB);
  mc = bmp085_read16(BMP085_CAL_MC);
  md = bmp085_read16(BMP085_CAL_MD);
#if (BMP085_DEBUG == 1)
  serial_printf("ac1 = %d",ac1);
  serial_printf("ac2 = %d",ac2);
  serial_printf("ac3 = %d",ac3);
  serial_printf("ac4 = %d",ac4);
  serial_printf("ac5 = %d",ac5);
  serial_printf("ac6 = %d",ac6);

  serial_printf("b1 = %d",b1);
  serial_printf("b2 = %d",b2);

  serial_printf("mb = %d",mb);
  serial_printf("mc = %d",mc);
  serial_printf("md = %d",md);
#endif

  return true;
}

int32_t ICACHE_FLASH_ATTR bmp085_computeB5(int32_t UT)
{
  int32_t X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15;
  int32_t X2 = ((int32_t)mc << 11) / (X1+(int32_t)md);
  return X1 + X2;
}

uint16_t ICACHE_FLASH_ATTR bmp085_readRawTemperature(void)
{
  bmp085_write8(BMP085_CONTROL, BMP085_READTEMPCMD);
  delay(5);
#if BMP085_DEBUG == 1
  serial_printf("Raw temp: %d", bmp085_read16(BMP085_TEMPDATA));
#endif
  return bmp085_read16(BMP085_TEMPDATA);
}

uint32_t ICACHE_FLASH_ATTR bmp085_readRawPressure(void)
{
  uint32_t raw;

  bmp085_write8(BMP085_CONTROL, BMP085_READPRESSURECMD + (bmp085_oversampling << 6));

  if (bmp085_oversampling == BMP085_ULTRALOWPOWER) 
    delay(5);
  else if (bmp085_oversampling == BMP085_STANDARD) 
    delay(8);
  else if (bmp085_oversampling == BMP085_HIGHRES) 
    delay(14);
  else 
    delay(26);

  raw = bmp085_read16(BMP085_PRESSUREDATA);

  raw <<= 8;
  raw |= bmp085_read8(BMP085_PRESSUREDATA+2);
  raw >>= (8 - bmp085_oversampling);

 /* this pull broke stuff, look at it later?
  if (bmp085_oversampling==0) {
    raw <<= 8;
    raw |= bmp085_read8(BMP085_PRESSUREDATA+2);
    raw >>= (8 - bmp085_oversampling);
  }
 */

#if BMP085_DEBUG == 1
  serial_printf("Raw pressure: %d\n", raw);
#endif
  return raw;
}


int32_t ICACHE_FLASH_ATTR bmp085_readPressure(void)
{
  int32_t UT, UP, B3, B5, B6, X1, X2, X3, p;
  uint32_t B4, B7;

  UT = bmp085_readRawTemperature();
  UP = bmp085_readRawPressure();

#if BMP085_DEBUG == 1
  // use datasheet numbers!
  UT = 27898;
  UP = 23843;
  ac6 = 23153;
  ac5 = 32757;
  mc = -8711;
  md = 2868;
  b1 = 6190;
  b2 = 4;
  ac3 = -14383;
  ac2 = -72;
  ac1 = 408;
  ac4 = 32741;
  bmp085_oversampling = 0;
#endif

  B5 = bmp085_computeB5(UT);

#if BMP085_DEBUG == 1
  Serial.printf("X1 = "); Serial.printfln(X1);
  Serial.printf("X2 = "); Serial.printfln(X2);
  Serial.printf("B5 = "); Serial.printfln(B5);
#endif

  // do pressure calcs
  B6 = B5 - 4000;
  X1 = ((int32_t)b2 * ( (B6 * B6)>>12 )) >> 11;
  X2 = ((int32_t)ac2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = ((((int32_t)ac1*4 + X3) << bmp085_oversampling) + 2) / 4;

#if BMP085_DEBUG == 1
  Serial.printf("B6 = "); Serial.printfln(B6);
  Serial.printf("X1 = "); Serial.printfln(X1);
  Serial.printf("X2 = "); Serial.printfln(X2);
  Serial.printf("B3 = "); Serial.printfln(B3);
#endif

  X1 = ((int32_t)ac3 * B6) >> 13;
  X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
  B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> bmp085_oversampling );

#if BMP085_DEBUG == 1
  Serial.printf("X1 = "); Serial.printfln(X1);
  Serial.printf("X2 = "); Serial.printfln(X2);
  Serial.printf("B4 = "); Serial.printfln(B4);
  Serial.printf("B7 = "); Serial.printfln(B7);
#endif

  if (B7 < 0x80000000) {
    p = (B7 * 2) / B4;
  } else {
    p = (B7 / B4) * 2;
  }
  X1 = (p >> 8) * (p >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * p) >> 16;

#if BMP085_DEBUG == 1
  Serial.printf("p = "); Serial.printfln(p);
  Serial.printf("X1 = "); Serial.printfln(X1);
  Serial.printf("X2 = "); Serial.printfln(X2);
#endif

  p = p + ((X1 + X2 + (int32_t)3791)>>4);
#if BMP085_DEBUG == 1
  serial_printf("p = %d\n", p); 
#endif
  return p;
}

int32_t ICACHE_FLASH_ATTR bmp085_readSealevelPressure(float altitude_meters)
{
  float pressure = bmp085_readPressure();
  return (int32_t)(pressure / pow(1.0-altitude_meters/44330, 5.255));
}

float ICACHE_FLASH_ATTR bmp085_readTemperature(void)
{
  int32_t UT, B5;     // following ds convention
  float temp;

  UT = bmp085_readRawTemperature();

#if BMP085_DEBUG == 1
  // use datasheet numbers!
  UT = 27898;
  ac6 = 23153;
  ac5 = 32757;
  mc = -8711;
  md = 2868;
#endif

  B5 = bmp085_computeB5(UT);
  temp = (B5+8) >> 4;
  temp /= 10;
  
  return temp;
}

float ICACHE_FLASH_ATTR bmp085_readAltitude(float sealevelPressure)
{
  float altitude;

  float pressure = bmp085_readPressure();

  altitude = 44330 * (1.0 - pow(pressure /sealevelPressure,0.1903));

  return altitude;
}

uint8_t ICACHE_FLASH_ATTR bmp085_read8(uint8_t a)
{
  uint8_t ret;

  wire_beginTransmission(BMP085_I2CADDR);	// start transmission to device 
  wire_write(a);							// sends register address to read from
  wire_endTransmission();					// end transmission
  
  wire_beginTransmission(BMP085_I2CADDR);	// start transmission to device 
  wire_requestFrom(BMP085_I2CADDR, 1);		// send data n-bytes read
  ret = wire_read();						// receive DATA
  wire_endTransmission();					// end transmission

  return ret;
}

uint16_t ICACHE_FLASH_ATTR bmp085_read16(uint8_t a)
{
  uint16_t ret;

  wire_beginTransmission(BMP085_I2CADDR);	// start transmission to device 
  wire_write(a);							// sends register address to read from
  wire_endTransmission();					// end transmission
  
  wire_beginTransmission(BMP085_I2CADDR);	// start transmission to device 
  wire_requestFrom(BMP085_I2CADDR, 2);		// send data n-bytes read
  ret = wire_read();						// receive DATA
  ret <<= 8;
  ret |= wire_read();						// receive DATA
  wire_endTransmission();					// end transmission

  return ret;
}

void ICACHE_FLASH_ATTR bmp085_write8(uint8_t a, uint8_t d)
{
  wire_beginTransmission(BMP085_I2CADDR);	// start transmission to device 
  wire_write(a);							// sends register address to read from
  wire_write(d);							// write data
  wire_endTransmission();					// end transmission
}
