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

#include "driver/uart.h"
#include "espnow.h"
#include "user_interface.h"

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
	os_bzero(recv_buf, 17);
	os_memcpy(recv_buf, data, len < 17 ? len : 16);
	os_printf(" len: %d, data: %s]:", len, recv_buf);

	if (os_strncmp(data, "ACK", 3) == 0)
		return;

	os_sprintf(ack_buf, "ACK[%08x]", ack_count++);
	esp_now_send(macaddr, ack_buf, os_strlen(ack_buf));
}

int ICACHE_FLASH_ATTR node_group_send(u8 * data, u8 len)
{
	int ret = 0;

	if (esp_now_is_peer_exist(slave_mac) == 0) {
		os_printf("The espnow peer does not exist\n");
		return 0;
	} else {
		/* the demo will send to two devices which added by esp_now_add_peer() */
		ret = esp_now_send(NULL, data, len);
		return ret;
	}
}

void ICACHE_FLASH_ATTR node_group_init(void)
{
	int ret = 0;

	if (esp_now_init() == 0) {
		os_printf("esp_now init ok\n");
		esp_now_register_recv_cb(simple_cb);

		u8 ch = wifi_get_channel();
		os_printf("current channel is: %d\n", ch);

		esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
		esp_now_add_peer(ctrl_mac, ESP_NOW_ROLE_CONTROLLER, ch, key, 16);
		esp_now_add_peer(slave_mac, ESP_NOW_ROLE_SLAVE, ch, key, 16);

	} else {
		os_printf("esp_now init failed\n");
	}
}

void ICACHE_FLASH_ATTR node_group_send_cb(void *arg)
{
	u8 all_cnt, encrypt_cnt;

	if (esp_now_get_cnt_info(&all_cnt, &encrypt_cnt)) {
		os_printf("get_cnt_info failed\r\n");
		os_printf("client:%d, encrypted client:%d\r\n", all_cnt, encrypt_cnt);
	}

	if (node_group_send("Hello", 6)) {
		os_printf("esp_now_send fail\r\n");
	} else {
		os_printf("esp_now_send ok\r\n");
	}
}

os_timer_t client_timer;

void user_init(void)
{
	uart_init(115200, 115200);

	wifi_set_opmode(STATION_MODE);
	wifi_set_macaddr(STATION_IF, ctrl_mac);

	node_group_init();

	// send data through espnow every 5s
	os_timer_setfn(&client_timer, (os_timer_func_t *)node_group_send_cb, NULL);
	os_timer_arm(&client_timer, 5000, 1);
}
