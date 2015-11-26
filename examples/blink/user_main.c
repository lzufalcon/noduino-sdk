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
#include "osapi.h"
#include "os_type.h"
#include "gpio.h"
#include "noduino.h"

static volatile os_timer_t blink_timer;

void gpio16_output()
{
	GPF16 = GP16FFS(GPFFS_GPIO(16));	//Set mode to GPIO
	GPC16 = 0;
	GP16E |= 1;
}

void gpio16_high()
{
	GP16O |= 1;
}

void gpio16_low()
{
	GP16O &= ~1;
}

void blink_timerfunc(void *arg)
{
	if (GPIO_INPUT_GET(2)) {
		//Current is HIGH, Set GPIO2 to LOW
		gpio_output_set(0, BIT2, BIT2, 0);
		gpio16_high();
	} else {
		//Current is LOW, Set GPIO2 to HIGH
		gpio_output_set(BIT2, 0, BIT2, 0);
		gpio16_low();
	}
}

//user_init is the user entry point of the Espressif SDK
void ICACHE_FLASH_ATTR user_init()
{
	//Initialize the GPIO subsystem.
	gpio_init();

	//Set GPIO2 to output mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

	//gpio16 output
	gpio16_output();
	gpio16_high();

	// Set GPIO2 high to disable the blue led
	// gpio2 low ---> blue led on
	gpio_output_set(0, BIT2, BIT2, 0);

	//Disable the timer
	os_timer_disarm(&blink_timer);

	//Setup timer
	os_timer_setfn(&blink_timer, (os_timer_func_t *) blink_timerfunc, NULL);

	//enable the timer
	//&blink_timer is the pointer
	//1200 is the fire time in ms
	//0 for once and 1 for repeating
	os_timer_arm(&blink_timer, 1200, 1);
}
