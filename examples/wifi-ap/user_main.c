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
#include "user_interface.h"
#include "driver/uart.h"

/*
 * struct softap_config {
 *   uint8 ssid[32];
 *   uint8 password[64];
 *   uint8 ssid_len;
 *   uint8 channel;
 *   uint8 authmode;
 *   uint8 ssid_hidden;
 *   uint8 max_connection;
 * }
 *
*/

// user entry
void ICACHE_FLASH_ATTR user_init()
{
	struct softap_config config;
	char ssid[] = "Noduino";

	// switch to softp ap mode
	wifi_set_opmode(SOFTAP_MODE);

	// get old softap config
	wifi_softap_get_config(&config);

	os_memcpy(config.ssid, ssid, os_strlen(ssid));
	config.ssid_len = os_strlen(ssid);

	// set the new ssid
	wifi_softap_set_config(&config);
}
