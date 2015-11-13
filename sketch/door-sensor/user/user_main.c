/*
 * Copyright 2015 MaiKe Labs
 *
 * Modification history:
 *     2015/11/13, create this file.
*/
#include "ets_sys.h"
#include "osapi.h"
#include "ip_addr.h"
#include "user_interface.h"
#include "espconn.h"
#include "os_type.h"

#include "driver/uart.h"
#include "airkiss.h"
#include "smartconfig.h"

#define	DEBUG	1

LOCAL uint8_t check_ip_count = 0;

os_timer_t time_serv;
os_timer_t client_timer;

esp_udp airkiss_udp;
struct espconn ptrairudpconn;

uint8_t lan_buf[200];
uint16_t lan_buf_len;

#define DEVICE_TYPE			"gh_95fae1ba6fa0"
#define DEVICE_ID			"gh_95fae1ba6fa0_8312db1c74a6d97d04063fb88d9a8e47"
#define DEFAULT_LAN_PORT	12476

const airkiss_config_t akconf = {
	(airkiss_memset_fn) & memset,
	(airkiss_memcpy_fn) & memcpy,
	(airkiss_memcmp_fn) & memcmp,
	0
};

static void time_callback(void)
{
	airkiss_udp.remote_port = DEFAULT_LAN_PORT;
	airkiss_udp.remote_ip[0] = 255;
	airkiss_udp.remote_ip[1] = 255;
	airkiss_udp.remote_ip[2] = 255;
	airkiss_udp.remote_ip[3] = 255;
	lan_buf_len = sizeof(lan_buf);

	int ret = airkiss_lan_pack(AIRKISS_LAN_SSDP_NOTIFY_CMD,
				   DEVICE_TYPE, DEVICE_ID, 0, 0, lan_buf,
				   &lan_buf_len, &akconf);

	if (ret != AIRKISS_LAN_PAKE_READY) {
		uart0_sendStr("Pack lan packet error!\r\n");
		return;
	}
	ret = espconn_sent(&ptrairudpconn, lan_buf, lan_buf_len);
	if (ret != 0) {
		uart0_sendStr("UDP send error!\r\n");

	}
	uart0_sendStr("Finish send notify!\r\n");
}

void wifilan_recv_callbk(void *arg, char *pdata, unsigned short len)
{
	airkiss_lan_ret_t ret = airkiss_lan_recv(pdata, len, &akconf);
	airkiss_lan_ret_t packret;

	switch (ret) {
	case AIRKISS_LAN_SSDP_REQ:

		airkiss_udp.remote_port = DEFAULT_LAN_PORT;
		lan_buf_len = sizeof(lan_buf);

		packret = airkiss_lan_pack(AIRKISS_LAN_SSDP_RESP_CMD,
					   DEVICE_TYPE, DEVICE_ID, 0, 0,
					   lan_buf, &lan_buf_len, &akconf);

		if (packret != AIRKISS_LAN_PAKE_READY) {
			uart0_sendStr("Pack lan packet error!\r\n");
			return;
		}

		packret = espconn_sent(&ptrairudpconn, lan_buf, lan_buf_len);

		if (packret != 0) {
			uart0_sendStr("LAN UDP Send err!\r\n");
		}
		break;
	default:
		uart0_sendStr("Pack is not ssdq req!\r\n");
		break;
	}
}

void ICACHE_FLASH_ATTR airkiss_nff_start(void)
{
	airkiss_udp.local_port = 12476;
	ptrairudpconn.type = ESPCONN_UDP;
	ptrairudpconn.proto.udp = &(airkiss_udp);

	espconn_create(&ptrairudpconn);
	espconn_regist_recvcb(&ptrairudpconn, wifilan_recv_callbk);

	os_timer_setfn(&time_serv, (os_timer_func_t *) time_callback, NULL);
	os_timer_arm(&time_serv, 5000, 1);	//5s定时器
}

void ICACHE_FLASH_ATTR airkiss_nff_stop(void)
{
	os_timer_disarm(&time_serv);
}

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

void ICACHE_FLASH_ATTR cos_check_ip()
{
	struct ip_info ipconfig;

	os_timer_disarm(&client_timer);

	wifi_get_ip_info(STATION_IF, &ipconfig);

	if (wifi_station_get_connect_status() == STATION_GOT_IP
		&& ipconfig.ip.addr != 0) {
		// connect to router success
		// TODO: notify the cloud I'm online
		// cloud return the bind state

		// start broadcast airkiss-nff udp pkg
		airkiss_nff_start();
	} else {
		// idle or connecting
		os_timer_setfn(&client_timer, (os_timer_func_t *)cos_check_ip, NULL);
		os_timer_arm(&client_timer, 100, 0);

		if (check_ip_count++ > 50) {
			// delay 10s, need to start airkiss to reconfig the network
			// TODO: flash led to show wifi disconnect
			smartconfig_set_type(SC_TYPE_AIRKISS);
			wifi_set_opmode(STATION_MODE);
			smartconfig_start(smartconfig_done);

			// reset the count
			check_ip_count = 0;
		}
	}
}

void ICACHE_FLASH_ATTR user_init(void)
{
#ifdef DEBUG
	uart_init(115200, 115200);
#endif

	system_init_done_cb(cos_check_ip);
}
