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
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

#include "espconn.h"

#define	SSID		"YOUR_ROUTER_SSID"
#define PASSWORD	"YOUR_ROUTER_PASSWD"

LOCAL os_timer_t test_timer;
LOCAL struct espconn user_udp_espconn;

const char *ESP8266_MSG = "I'm ESP8266 ";

LOCAL struct espconn ptrespconn;

LOCAL void ICACHE_FLASH_ATTR
user_udp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	os_printf("recv udp data: %s\n", pusrdata);
}

LOCAL void ICACHE_FLASH_ATTR user_udp_send(void)
{
	char DeviceBuffer[40] = { 0 };
	char hwaddr[6];
	struct ip_info ipconfig;

	const char udp_remote_ip[4] = { 255, 255, 255, 255 };

	// ESP8266 udp remote IP need to be set everytime we call espconn_sent
	os_memcpy(user_udp_espconn.proto.udp->remote_ip, udp_remote_ip, 4);

	// ESP8266 udp remote port need to be set everytime we call espconn_sent
	user_udp_espconn.proto.udp->remote_port = 1112;

	wifi_get_macaddr(STATION_IF, hwaddr);

	os_sprintf(DeviceBuffer, "%s" MACSTR "!", ESP8266_MSG, MAC2STR(hwaddr));

	espconn_sent(&user_udp_espconn, DeviceBuffer, os_strlen(DeviceBuffer));
}

LOCAL void ICACHE_FLASH_ATTR user_udp_sent_cb(void *arg)
{
	struct espconn *pespconn = arg;

	os_printf("user_udp_send successfully !!!\n");

	//disarm timer first
	os_timer_disarm(&test_timer);

	// re-arm timer to check ip
	// only send next packet after prev packet sent successfully
	os_timer_setfn(&test_timer, (os_timer_func_t *) user_udp_send, NULL);
	os_timer_arm(&test_timer, 1000, 0);
}

void ICACHE_FLASH_ATTR user_check_ip(void)
{
	struct ip_info ipconfig;

	//disarm timer first
	os_timer_disarm(&test_timer);

	//get ip info of ESP8266 station
	wifi_get_ip_info(STATION_IF, &ipconfig);

	if (wifi_station_get_connect_status() == STATION_GOT_IP
	    && ipconfig.ip.addr != 0) {
		os_printf("got ip !!! \r\n");

		// send UDP broadcast from both station and soft-AP interface
		wifi_set_broadcast_if(STATIONAP_MODE);

		user_udp_espconn.type = ESPCONN_UDP;
		user_udp_espconn.proto.udp =
		    (esp_udp *) os_zalloc(sizeof(esp_udp));

		// set a available  port
		user_udp_espconn.proto.udp->local_port = espconn_port();

		const char udp_remote_ip[4] = { 255, 255, 255, 255 };

		// ESP8266 udp remote IP
		os_memcpy(user_udp_espconn.proto.udp->remote_ip, udp_remote_ip, 4);

		// ESP8266 udp remote port
		user_udp_espconn.proto.udp->remote_port = 1112;

		// register a udp packet receiving callback
		espconn_regist_recvcb(&user_udp_espconn, user_udp_recv_cb);

		// register a udp packet sent callback
		espconn_regist_sentcb(&user_udp_espconn, user_udp_sent_cb);

		// create udp
		espconn_create(&user_udp_espconn);

		// send udp data
		user_udp_send();

	} else {
		if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD
		     || wifi_station_get_connect_status() == STATION_NO_AP_FOUND
		     || wifi_station_get_connect_status() ==
		     STATION_CONNECT_FAIL)) {
			os_printf("connect fail !!! \r\n");
		} else {
			//re-arm timer to check ip
			os_timer_setfn(&test_timer,
				       (os_timer_func_t *) user_check_ip, NULL);
			os_timer_arm(&test_timer, 100, 0);
		}
	}
}

void ICACHE_FLASH_ATTR user_set_station_config(void)
{
	// Wifi configuration
	char ssid[32] = SSID;
	char password[64] = PASSWORD;
	struct station_config stationConf;

	//need not mac address
	stationConf.bssid_set = 0;

	//Set ap settings
	os_memcpy(&stationConf.ssid, ssid, 32);
	os_memcpy(&stationConf.password, password, 64);
	wifi_station_set_config(&stationConf);

	//set a timer to check whether got ip from router succeed or not.
	os_timer_disarm(&test_timer);
	os_timer_setfn(&test_timer, (os_timer_func_t *) user_check_ip, NULL);
	os_timer_arm(&test_timer, 100, 0);
}

void user_init(void)
{
	os_printf("SDK version:%s\n", system_get_sdk_version());

	//Set softAP + station mode
	wifi_set_opmode(STATIONAP_MODE);

	//ESP8266 connect to router
	user_set_station_config();
}
