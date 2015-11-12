#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "ip_addr.h"
#include "user_interface.h"
#include "espconn.h"
#include "os_type.h"

#include "airkiss.h"

//用于发送上线通知包的定时器,平台相关
os_timer_t time_serv;

//UDP套接字相关变量,平台相关
esp_udp airkiss_udp;
struct espconn ptrairudpconn;

//用于缓存回包的数据缓冲区,也可以为局部变量
uint8_t lan_buf[200];
uint16_t lan_buf_len;

#define DEVICE_TYPE			"gh_95fae1ba6fa0"
#define DEVICE_ID			"gh_95fae1ba6fa0_8312db1c74a6d97d04063fb88d9a8e47"
#define DEFAULT_LAN_PORT	12476

//定义AirKiss库需要用到的一些标准函数,由对应的硬件平台提供,前三个为必要函数
const airkiss_config_t akconf = {
	(airkiss_memset_fn) & memset,
	(airkiss_memcpy_fn) & memcpy,
	(airkiss_memcmp_fn) & memcmp,
	0
};

/*
* 平台相关定时器中断处理函数,比较正确的做法是在中断里面发送信号通知任务发送,这里
* 为了方便说明直接发送
*/
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

/*
* 硬件平台相关,UDP监听端口数据接收处理函数
*/
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

/*
* 硬件平台相关,创建UDP套接字,监听12476端口,与AirKiss库使用无关
*/
static void mm_startlandiscover(void)
{
	airkiss_udp.local_port = 12476;
	ptrairudpconn.type = ESPCONN_UDP;
	ptrairudpconn.proto.udp = &(airkiss_udp);

	espconn_create(&ptrairudpconn);
	espconn_regist_recvcb(&ptrairudpconn, wifilan_recv_callbk);

	os_timer_setfn(&time_serv, (os_timer_func_t *) time_callback, NULL);
	os_timer_arm(&time_serv, 5000, 1);	//5s定时器
}

/*
* 硬件平台初始化,与AirKiss库使用无关
*/
void ICACHE_FLASH_ATTR user_init(void)
{
	uart_init(115200, 115200);
	//这里假设设备已经连接上WiFi了,直接开始创建UDP

	system_init_done_cb(mm_startlandiscover);
}
