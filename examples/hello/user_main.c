/*
 * Copyright 2015 MaiKe Labs
 *
 * Description: a simple example to print 'Hello world!'
 *
 * Modification history:
 *     2015/11/10, create this file.
 *
*/
#include "osapi.h"
#include "os_type.h"

#include "driver/uart.h"

static volatile os_timer_t hello_timer;

void hello_timerfunc(void *arg)
{
	os_printf("%s\n", "Hello World!");
}

//user_init is the user entry point of the Espressif SDK
void ICACHE_FLASH_ATTR user_init()
{
	//Initialize the uart0 and uart1 in 115200 bitrate
	uart_init(BIT_RATE_115200, BIT_RATE_115200);

	//Disable the timer
	os_timer_disarm(&hello_timer);

	//Setup timer
	os_timer_setfn(&hello_timer, (os_timer_func_t *) hello_timerfunc, NULL);

	//enable the timer
	//&hello_timer is the pointer
	//2000 is the fire time in ms
	//0 for once and 1 for repeating
	os_timer_arm(&hello_timer, 2000, 1);
}
