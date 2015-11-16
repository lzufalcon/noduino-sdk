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
#include "user_interface.h"

#include "driver/key.h"

#define WPS_KEY_NUM        1

#define WPS_KEY_IO_MUX     PERIPHS_IO_MUX_MTCK_U
#define WPS_KEY_IO_NUM     13
#define WPS_KEY_IO_FUNC    FUNC_GPIO13

LOCAL struct keys_param keys;
LOCAL struct single_key_param *single_key;

LOCAL void ICACHE_FLASH_ATTR
user_wps_status_cb(int status)
{
	switch (status) {
		case WPS_CB_ST_SUCCESS:
			wifi_wps_disable();
			wifi_station_connect();
			break;
		case WPS_CB_ST_FAILED:
		case WPS_CB_ST_TIMEOUT:
			wifi_wps_start();
			break;
	}
}

LOCAL void ICACHE_FLASH_ATTR
user_wps_key_short_press(void)
{
	wifi_wps_disable();
	wifi_wps_enable(WPS_TYPE_PBC);
	wifi_set_wps_cb(user_wps_status_cb);
	wifi_wps_start();
}

void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}

void ICACHE_FLASH_ATTR
user_init(void)
{
	single_key = key_init_single(WPS_KEY_IO_NUM, WPS_KEY_IO_MUX, WPS_KEY_IO_FUNC,
		                                    NULL, user_wps_key_short_press);

	keys.key_num = WPS_KEY_NUM;
	keys.single_key = &single_key;

	key_init(&keys);

	wifi_set_opmode(STATION_MODE);
}
