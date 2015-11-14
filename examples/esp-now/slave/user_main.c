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

void ICACHE_FLASH_ATTR demo_send_(u8 * data, u8 len)
{
	/* the demo will send to two devices which added by esp_now_add_peer() */
	esp_now_send(NULL, data, len);

}

void ICACHE_FLASH_ATTR node_group_init(void)
{
	u8 key[16] = { 0x33, 0x44, 0x33, 0x44, 0x33, 0x44, 0x33, 0x44, 0x33,
		0x44, 0x33, 0x44, 0x33, 0x44, 0x33, 0x44};
	u8 da1[6] = { 0x18, 0xfe, 0x34, 0x97, 0xd5, 0xb1 };
	u8 da2[6] = { 0x1a, 0xfe, 0x34, 0x97, 0xd5, 0xb1 };

	if (esp_now_init() == 0) {
		os_printf("esp_now init ok\n");

		u8 ch = wifi_get_channel();
		os_printf("dlink send to A cur chan %d\n", ch);

		esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
		//esp_now_add_peer(da1, ESP_NOW_ROLE_CONTROLLER, key, 16);
		//esp_now_add_peer(da2, ESP_NOW_ROLE_SLAVE, key, 16)
	} else {
		os_printf("esp_now init failed\n");
	}
}

void user_init(void)
{
	uart_init(115200, 115200);

	wifi_set_opmode(SOFTAP_MODE);

	node_group_init();
}
