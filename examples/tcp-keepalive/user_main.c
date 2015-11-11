/*
 * Function: Set option of TCP connection
 * 
 * Prototype:
 * 
 * 	int8 espconn_set_opt(
 * 			struct espconn *espconn,
 * 			uint8 opt
 * 	)
 * 
 * Parameter:
 * 
 * 	struct espconn *espconn : corresponding connected control structure
 * 	uint8 opt : Option of TCP connection
 *		bit 0: 1: free memory after TCP disconnection happen need not wait 2 minutes;
 * 		bit 1: 1: disable nalgo algorithm during TCP data transmission, quiken the data transmission.
 * 		bit 2: 1: enable espconn_write_finish_callback which means data is written into the 2920 bytes write buffer and wait for sending.
 * 		bit 3: 1: enable TCP keep-alive function
 * 
 * Return:
 * 	0      : succeed
 * 	Non-0  : error (please refer to espconn.h for details.)
 * 	   e.g. ESPCONN_ARG: illegal argument，can’t find TCP connection according to structure espconn
 * 
 * Note:
 *		In general, we need not call this API;
 *		If call espconn_set_opt, please call it in TCP connected callback.
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
LOCAL struct espconn user_tcp_conn;
LOCAL struct _esp_tcp user_tcp;
ip_addr_t tcp_server_ip;

// remote IP of tcp server
const char esp_server_ip[4] = { 1, 1, 1, 1 };
#define	REMOTE_PORT			80

LOCAL void ICACHE_FLASH_ATTR
user_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	//received some data from tcp connection
	os_printf("tcp recv !!! %s \r\n", pusrdata);
}

LOCAL void ICACHE_FLASH_ATTR user_tcp_sent_cb(void *arg)
{
	//data sent successfully
}

LOCAL void ICACHE_FLASH_ATTR user_tcp_discon_cb(void *arg)
{
	//tcp disconnect successfully
	os_printf("tcp disconnect succeed !!! \r\n");
}

LOCAL void ICACHE_FLASH_ATTR user_tcp_write_finish(void *arg)
{
	struct espconn *pespconn = arg;
	espconn_sent(pespconn, "Hello World!", 12);
}

LOCAL void ICACHE_FLASH_ATTR user_sent_data(struct espconn *pespconn)
{
	espconn_sent(pespconn, "Hello World!", 12);
}

LOCAL void ICACHE_FLASH_ATTR user_tcp_connect_cb(void *arg)
{
	struct espconn *pespconn = arg;

	os_printf("connect succeed !!! \r\n");

	espconn_regist_recvcb(pespconn, user_tcp_recv_cb);
	espconn_regist_sentcb(pespconn, user_tcp_sent_cb);
	espconn_regist_disconcb(pespconn, user_tcp_discon_cb);

	// enable write buffer
	espconn_set_opt(pespconn, 0x04);

	// register write finish callback
	espconn_regist_write_finish(pespconn, user_tcp_write_finish);

	user_sent_data(pespconn);
}

LOCAL void ICACHE_FLASH_ATTR user_tcp_recon_cb(void *arg, sint8 err)
{
	//error occured , tcp connection broke. user can try to reconnect here.
	os_printf("reconnect callback, error code %d !!! \r\n", err);
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

		// Connect to tcp server as NET_DOMAIN
		user_tcp_conn.proto.tcp = &user_tcp;
		user_tcp_conn.type = ESPCONN_TCP;
		user_tcp_conn.state = ESPCONN_NONE;

		// remote ip of tcp server
		os_memcpy(user_tcp_conn.proto.tcp->remote_ip, esp_server_ip, 4);

		// remote port of tcp server
		user_tcp_conn.proto.tcp->remote_port = REMOTE_PORT;

		//local port of ESP8266
		user_tcp_conn.proto.tcp->local_port = espconn_port();

		// register connect callback
		espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb);
		// register reconnect callback as error handler
		espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb);

		// tcp connect
		espconn_connect(&user_tcp_conn);
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
