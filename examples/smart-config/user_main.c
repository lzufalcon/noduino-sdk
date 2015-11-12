/*
 * Copyright 2015 MaiKe Labs
 *
 * Modification history:
 *     2015/11/10, create this file.
*/

#include "ets_sys.h"
#include "osapi.h"

#include "driver/uart.h"
#include "user_interface.h"
#include "smartconfig.h"

void ICACHE_FLASH_ATTR smartconfig_done(sc_status status, void *pdata)
{
	switch (status) {
	case SC_STATUS_WAIT:
		os_printf("SC_STATUS_WAIT\n");
		break;
	case SC_STATUS_FIND_CHANNEL:
		os_printf("SC_STATUS_FIND_CHANNEL\n");
		break;
	case SC_STATUS_GETTING_SSID_PSWD:
		os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
		sc_type *type = pdata;
		if (*type == SC_TYPE_ESPTOUCH) {
			os_printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
		} else {
			os_printf("SC_TYPE:SC_TYPE_AIRKISS\n");
		}
		break;
	case SC_STATUS_LINK:
		os_printf("SC_STATUS_LINK\n");
		struct station_config *sta_conf = pdata;

		os_printf("Store the ssid and password into flash\n");
		wifi_station_set_config(sta_conf);

		wifi_station_disconnect();
		wifi_station_connect();
		break;
	case SC_STATUS_LINK_OVER:
		os_printf("SC_STATUS_LINK_OVER\n");
		if (pdata != NULL) {
			uint8 phone_ip[4] = { 0 };

			os_memcpy(phone_ip, (uint8 *) pdata, 4);
			os_printf("Phone ip: %d.%d.%d.%d\n", phone_ip[0],
				  phone_ip[1], phone_ip[2], phone_ip[3]);
		}
		smartconfig_stop();
		break;
	}

}

void user_rf_pre_init(void)
{

}

void user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);

	os_printf("SDK version:%s\n", system_get_sdk_version());

	//SC_TYPE_ESPTOUCH,SC_TYPE_AIRKISS,SC_TYPE_ESPTOUCH_AIRKISS
	smartconfig_set_type(SC_TYPE_AIRKISS);

	wifi_set_opmode(STATION_MODE);

	smartconfig_start(smartconfig_done);
}
