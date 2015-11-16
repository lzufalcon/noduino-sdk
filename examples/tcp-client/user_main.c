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

#define NET_DOMAIN "cn.bing.com"
#define pheadbuffer "GET / HTTP/1.1\r\nUser-Agent: curl/7.37.0\r\nHost: %s\r\nAccept: */*\r\n\r\n"

#define packet_size		(2 * 1024)

#define	REMOTE_PORT		80
#define	SSID			"YOUR_ROUTER_SSID"
#define PASSWORD		"YOUR_ROUTER_PASSWD"

LOCAL os_timer_t test_timer;
LOCAL struct espconn user_tcp_conn;
LOCAL struct _esp_tcp user_tcp;
ip_addr_t tcp_server_ip;

// remote IP of TCP server
const char remote_server_ip[4] = { 1, 1, 1, 1 };

LOCAL void ICACHE_FLASH_ATTR
user_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	//received some data from tcp connection
	os_printf("tcp recv !!! %s \r\n", pusrdata);
}

LOCAL void ICACHE_FLASH_ATTR user_tcp_sent_cb(void *arg)
{
	//data sent successfully
	os_printf("tcp sent succeed !!! \r\n");
}

LOCAL void ICACHE_FLASH_ATTR user_tcp_discon_cb(void *arg)
{
	//tcp disconnect successfully
	os_printf("tcp disconnect succeed !!! \r\n");
}

LOCAL void ICACHE_FLASH_ATTR user_sent_data(struct espconn *pespconn)
{
	char *pbuf = (char *)os_zalloc(packet_size);

	os_sprintf(pbuf, pheadbuffer, NET_DOMAIN);

	espconn_sent(pespconn, pbuf, os_strlen(pbuf));

	os_free(pbuf);

}

LOCAL void ICACHE_FLASH_ATTR user_tcp_connect_cb(void *arg)
{
	struct espconn *pespconn = arg;

	os_printf("connect succeed !!! \r\n");

	espconn_regist_recvcb(pespconn, user_tcp_recv_cb);
	espconn_regist_sentcb(pespconn, user_tcp_sent_cb);
	espconn_regist_disconcb(pespconn, user_tcp_discon_cb);

	user_sent_data(pespconn);
}

LOCAL void ICACHE_FLASH_ATTR user_tcp_recon_cb(void *arg, sint8 err)
{
	//error occured , tcp connection broke. user can try to reconnect here.
	os_printf("reconnect callback, error code %d !!! \r\n", err);
}

#ifdef DNS_ENABLE
LOCAL void ICACHE_FLASH_ATTR
user_dns_found(const char *name, ip_addr_t * ipaddr, void *arg)
{
	struct espconn *pespconn = (struct espconn *)arg;

	if (ipaddr == NULL) {
		os_printf("user_dns_found NULL \r\n");
		return;
	}
	//dns got ip
	os_printf("user_dns_found %d.%d.%d.%d \r\n",
		  *((uint8 *) & ipaddr->addr), *((uint8 *) & ipaddr->addr + 1),
		  *((uint8 *) & ipaddr->addr + 2),
		  *((uint8 *) & ipaddr->addr + 3));

	if (tcp_server_ip.addr == 0 && ipaddr->addr != 0) {
		// dns succeed, create tcp connection
		os_timer_disarm(&test_timer);
		tcp_server_ip.addr = ipaddr->addr;

		// remote ip of tcp server which get by dns
		os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);

		// remote port of tcp server
		pespconn->proto.tcp->remote_port = 80;

		//local port of ESP8266
		pespconn->proto.tcp->local_port = espconn_port();

		// register connect callback
		espconn_regist_connectcb(pespconn, user_tcp_connect_cb);

		// register reconnect callback as error handler
		espconn_regist_reconcb(pespconn, user_tcp_recon_cb);

		// tcp connect
		espconn_connect(pespconn);
	}
}

LOCAL void ICACHE_FLASH_ATTR user_dns_check_cb(void *arg)
{
	struct espconn *pespconn = arg;

	// recall DNS function
	espconn_gethostbyname(pespconn, NET_DOMAIN, &tcp_server_ip, user_dns_found);

	os_timer_arm(&test_timer, 1000, 0);
}
#endif

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

		// Connect to tcp server as NET_DOMAIN
		user_tcp_conn.proto.tcp = &user_tcp;
		user_tcp_conn.type = ESPCONN_TCP;
		user_tcp_conn.state = ESPCONN_NONE;

#ifdef DNS_ENABLE
		tcp_server_ip.addr = 0;

		// DNS function
		espconn_gethostbyname(&user_tcp_conn, NET_DOMAIN,
					&tcp_server_ip, user_dns_found);

		os_timer_setfn(&test_timer,
					(os_timer_func_t *) user_dns_check_cb,
					user_tcp_conn);
		os_timer_arm(&test_timer, 1000, 0);
#else
		os_memcpy(user_tcp_conn.proto.tcp->remote_ip, remote_server_ip, 4);

		// remote port
		user_tcp_conn.proto.tcp->remote_port = REMOTE_PORT;

		//local port of ESP8266
		user_tcp_conn.proto.tcp->local_port = espconn_port();

		// register connect callback
		espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb);
		// register reconnect callback as error handler
		espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb);
		espconn_connect(&user_tcp_conn);

#endif
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
