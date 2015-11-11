#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

#include "espconn.h"

const char *device_find_request = "Are You ESP8266 Device?";
const char *device_find_response_ok = "Yes,I'm ESP8266!";

LOCAL struct espconn ptrespconn;

LOCAL void ICACHE_FLASH_ATTR
user_udp_recv(void *arg, char *pusrdata, unsigned short length)
{
	char DeviceBuffer[40] = { 0 };
	char Device_mac_buffer[60] = { 0 };
	char hwaddr[6];

	struct ip_info ipconfig;

	if (wifi_get_opmode() != STATION_MODE) {
		wifi_get_ip_info(SOFTAP_IF, &ipconfig);
		wifi_get_macaddr(SOFTAP_IF, hwaddr);

		if (!ip_addr_netcmp
		    ((struct ip_addr *)ptrespconn.proto.udp->remote_ip,
		     &ipconfig.ip, &ipconfig.netmask)) {
			//udp packet is received from ESP8266 station
			wifi_get_ip_info(STATION_IF, &ipconfig);
			wifi_get_macaddr(STATION_IF, hwaddr);
		} else {
			//udp packet is received from ESP8266 softAP
		}

	} else {
		//udp packet is received from ESP8266 station
		wifi_get_ip_info(STATION_IF, &ipconfig);
		wifi_get_macaddr(STATION_IF, hwaddr);
	}

	if (pusrdata == NULL)
		return;

	if (length == os_strlen(device_find_request) &&
	    os_strncmp(pusrdata, device_find_request,
		       os_strlen(device_find_request)) == 0) {
		//received device find message

		os_sprintf(DeviceBuffer, "%s" MACSTR " " IPSTR,
			   device_find_response_ok, MAC2STR(hwaddr),
			   IP2STR(&ipconfig.ip));

		os_printf("%s\n", DeviceBuffer);
		length = os_strlen(DeviceBuffer);

		//if received "Are You ESP8266 ?" ,
		//response "Yes,I'm ESP8266!" + ESP8266 mac + ESP8266 ip
		espconn_sent(&ptrespconn, DeviceBuffer, length);

	} else {
		//received some other data
	}

}

void ICACHE_FLASH_ATTR user_udp_init(void)
{
	ptrespconn.type = ESPCONN_UDP;
	ptrespconn.proto.udp = (esp_udp *) os_zalloc(sizeof(esp_udp));
	ptrespconn.proto.udp->local_port = 1025;	// ESP8266 udp port
	espconn_regist_recvcb(&ptrespconn, user_udp_recv);	// register a udp packet receiving callback
	espconn_create(&ptrespconn);	// create udp
}

void user_init(void)
{
	os_printf("SDK version:%s\n", system_get_sdk_version());

	//Set softAP + station mode
	wifi_set_opmode(STATIONAP_MODE);

	// Create udp listening.
	user_udp_init();
}
