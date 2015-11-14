/*
 * Copyright 2015 MaiKe Labs
 *
 * Description: a simple example to show how to
 * use the esp now
 *
 * Modification history:
 *     2015/11/14, create this file.
 *
*/
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "espnow.h"

int ack_count = 0;

u8 key[16] = { 0x33, 0x44, 0x33, 0x44, 0x33, 0x44, 0x33, 0x44, 0x33,
	0x44, 0x33, 0x44, 0x33, 0x44, 0x33, 0x44};

u8 ctrl_mac[6] = { 0x18, 0xfe, 0x34, 0xf9, 0x0f, 0x17 };
u8 slave_mac[6] = { 0x1a, 0xfe, 0x34, 0xf3, 0x73, 0xfc };

void ICACHE_FLASH_ATTR simple_cb(u8 * macaddr, u8 * data, u8 len)
{
	int i;
	u8 ack_buf[16];
	u8 recv_buf[17];
	os_printf("now from[");
	for (i = 0; i < 6; i++)
		os_printf("%02X, ", macaddr[i]);
	os_printf(" len: %d]:", len);
	os_bzero(recv_buf, 17);
	os_memcpy(recv_buf, data, len < 17 ? len : 16);

	//show_buf2(data, len);
	if (os_strncmp(data, "ACK", 3) == 0)
		return;

	os_sprintf(ack_buf, "ACK[%08x]", ack_count++);
	esp_now_send(macaddr, ack_buf, os_strlen(ack_buf));
}

int ICACHE_FLASH_ATTR demo_send(u8 * data, u8 len)
{
	if (esp_now_is_peer_exist(slave_mac) == 0) {
		os_printf("The espnow peer does not exist\n");
		return 0;
	} else {

		u8 old_ch = wifi_get_channel();

		u8 s_ch = esp_now_get_peer_channel(slave_mac);
		os_printf("before set: slave ch: %d\n", s_ch);

		u8 c_ch = esp_now_get_peer_channel(ctrl_mac);
		os_printf("before espnow send: ctrl ch: %d\n",c_ch);

		esp_now_set_peer_channel(slave_mac, c_ch);
		//wifi_set_channel(s_ch);
		
		s_ch = esp_now_get_peer_channel(slave_mac);
		os_printf("after set: slave ch: %d\n", s_ch);

		/* the demo will send to two devices which added by esp_now_add_peer() */
		int ret = esp_now_send(NULL, data, len);

		wifi_set_channel(old_ch);
		return ret;
	}
}

void ICACHE_FLASH_ATTR espnow_check_cb(void *arg)
{
	u8 all_cnt, encrypt_cnt;

	if (esp_now_get_cnt_info(&all_cnt, &encrypt_cnt)) {
		os_printf("get_cnt_info failed\r\n");
		os_printf("client:%d, encrypted client:%d\r\n", all_cnt, encrypt_cnt);
	}

	if (demo_send("Hello", 6)) {
		os_printf("esp_now_send fail\r\n");
	} else {
		os_printf("esp_now_send ok\r\n");
	}
}

void ICACHE_FLASH_ATTR node_group_init(void)
{
	if (esp_now_init() == 0) {
		os_printf("esp_now init ok\n");

		u8 ch = wifi_get_channel();
		os_printf("dlink send to A cur chan %d\n", ch);

		esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
		esp_now_add_peer(ctrl_mac, ESP_NOW_ROLE_CONTROLLER, ch, key, 16);
		esp_now_add_peer(slave_mac, ESP_NOW_ROLE_SLAVE, ch, key, 16)
	} else {
		os_printf("esp_now init failed\n");
	}
}

void user_init(void)
{
	uart_init(115200, 115200);

	wifi_set_opmode(SOFTAP_MODE);
	wifi_set_macaddr(SOFTAP_IF, slave_mac);

	node_group_init();
}
