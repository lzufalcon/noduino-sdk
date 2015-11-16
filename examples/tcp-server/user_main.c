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

LOCAL void ICACHE_FLASH_ATTR tcp_server_sent_cb(void *arg)
{
	//data sent successfully

	os_printf("tcp sent cb \r\n");
}

LOCAL void ICACHE_FLASH_ATTR
tcp_server_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	//received some data from tcp connection

	struct espconn *pespconn = arg;
	os_printf("tcp recv : %s \r\n", pusrdata);

	espconn_sent(pespconn, pusrdata, length);

}

LOCAL void ICACHE_FLASH_ATTR tcp_server_discon_cb(void *arg)
{
	//tcp disconnect successfully

	os_printf("tcp disconnect succeed !!! \r\n");
}

LOCAL void ICACHE_FLASH_ATTR tcp_server_recon_cb(void *arg, sint8 err)
{
	//error occured , tcp connection broke.

	os_printf("reconnect callback, error code %d !!! \r\n", err);
}

LOCAL void ICACHE_FLASH_ATTR tcp_server_listen(void *arg)
{
	struct espconn *pesp_conn = arg;
	os_printf("tcp_server_listen !!! \r\n");

	espconn_regist_recvcb(pesp_conn, tcp_server_recv_cb);
	espconn_regist_reconcb(pesp_conn, tcp_server_recon_cb);
	espconn_regist_disconcb(pesp_conn, tcp_server_discon_cb);

	espconn_regist_sentcb(pesp_conn, tcp_server_sent_cb);
}

void ICACHE_FLASH_ATTR user_tcpserver_init(uint32 port)
{
	LOCAL struct espconn esp_conn;
	LOCAL esp_tcp esptcp;

	esp_conn.type = ESPCONN_TCP;
	esp_conn.state = ESPCONN_NONE;
	esp_conn.proto.tcp = &esptcp;
	esp_conn.proto.tcp->local_port = port;
	espconn_regist_connectcb(&esp_conn, tcp_server_listen);

	sint8 ret = espconn_accept(&esp_conn);

	os_printf("espconn_accept [%d] !!! \r\n", ret);

}

LOCAL os_timer_t test_timer;

void ICACHE_FLASH_ATTR user_esp_platform_check_ip(void)
{
	struct ip_info ipconfig;

	//disarm timer first
	os_timer_disarm(&test_timer);

	//get ip info of ESP8266 station
	wifi_get_ip_info(STATION_IF, &ipconfig);

	if (wifi_station_get_connect_status() == STATION_GOT_IP
	    && ipconfig.ip.addr != 0) {

		os_printf("got ip !!! \r\n");

		user_tcpserver_init(1112);

	} else {

		if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD
		     || wifi_station_get_connect_status() == STATION_NO_AP_FOUND
		     || wifi_station_get_connect_status() ==
		     STATION_CONNECT_FAIL)) {

			os_printf("connect fail !!! \r\n");

		} else {

			//re-arm timer to check ip
			os_timer_setfn(&test_timer, (os_timer_func_t *)
				       user_esp_platform_check_ip, NULL);
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
	os_timer_setfn(&test_timer,
		       (os_timer_func_t *) user_esp_platform_check_ip, NULL);
	os_timer_arm(&test_timer, 100, 0);

}

void user_init(void)
{
	os_printf("SDK version:%s\n", system_get_sdk_version());

	//Set  station mode
	wifi_set_opmode(STATION_MODE);

	// ESP8266 connect to router.
	user_set_station_config();
}
