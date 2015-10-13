/*
 * user_main.c
 *
 *  Created on: Nov 15, 2014
 *      Author: frans-willem
 */
#include <sdkfixup.h>
extern "C" {
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
}
#include "input_protocols/tpm2net.h"
#include "output_protocols/ws2801.h"
#include "output_protocols/ws2812.h"
#include "input_protocols/artnet.h"
#include "input_protocols/tpm2net.h"
#include "config/config.h"
#include "debug/CDebugServer.h"
#include "httpd/CHttpServer.h"
#include <string.h>

unsigned int nWifiMode;
char szWifiSsid[32];
char szWifiPassword[64];
bool bUseDhcp;
uint32_t aIP;
uint32_t aNetmask;
uint32_t aGateway;

BEGIN_CONFIG(wifi, "WiFi");
CONFIG_SELECTSTART("mode","Operation mode", &nWifiMode, SOFTAP_MODE);
CONFIG_SELECTOPTION("Access point", SOFTAP_MODE);
CONFIG_SELECTOPTION("Client", STATION_MODE);
CONFIG_SELECTEND();
CONFIG_STRING("ssid", "SSID", szWifiSsid, sizeof(szWifiSsid)-1, "EspLightNode");
CONFIG_STRING("password","Password", szWifiPassword, sizeof(szWifiPassword)-1,"");
CONFIG_BOOLEAN("dhcp","Automatic configuration (DHCP)", &bUseDhcp, true);
CONFIG_IP("ip","IP Address", &aIP, 192, 168, 1, 1);
CONFIG_IP("netmask","Netmask", &aNetmask, 255, 255, 255, 0);
CONFIG_IP("gateway","Gateway", &aGateway, 192, 168, 1, 1);
END_CONFIG();

void start_services() {
	CHttpServer *pServer = new CHttpServer(80);
	
	config_init(pServer);
#ifdef ENABLE_TPM2
	tpm2net_init();
#endif
#ifdef ENABLE_ARTNET
	artnet_init();
#endif
}
static os_timer_t client_timer;
static void ICACHE_FLASH_ATTR wait_for_ip(uint8 flag) {
    LOCAL struct ip_info ipconfig;
    LOCAL int status;
    os_timer_disarm(&client_timer);

    status = wifi_station_get_connect_status();
    if (status == STATION_GOT_IP) {
        wifi_get_ip_info(STATION_IF, &ipconfig);
        if( ipconfig.ip.addr != 0) {
        	//Start UDP server
		DEBUG("Started %s", "OK");
			start_services();
        } else {
            os_timer_setfn(&client_timer, (os_timer_func_t *)wait_for_ip, NULL);
            os_timer_arm(&client_timer, 100, 0);
        }
    } else if (status == STATION_CONNECTING) {
        os_timer_setfn(&client_timer, (os_timer_func_t *)wait_for_ip, NULL);
        os_timer_arm(&client_timer, 100, 0);
    } else { //STATION_NO_AP_FOUND||STATION_CONNECT_FAIL||STATION_WRONG_PASSWORD
    	//Connection failed, somehow :(
        //system_restart();
        //TODO: Bring up SOFTAP here?
    }
}

static void ICACHE_FLASH_ATTR system_is_done(void){
	//Bringing up WLAN
	if (nWifiMode == STATION_MODE) {
		wifi_station_connect();
		//Wait for connection
		os_timer_disarm(&client_timer);
		os_timer_setfn(&client_timer, (os_timer_func_t *)wait_for_ip, NULL);
		os_timer_arm(&client_timer, 100, 0);
	} else if (nWifiMode == SOFTAP_MODE) {
		start_services();
	}
}

//Init function
extern "C" void ICACHE_FLASH_ATTR
user_init()
{
	config_load();
	wifi_set_opmode(NULL_MODE); //Next time start up in NULL mode
	wifi_set_opmode_current(NULL_MODE);

	//Initialize station config parameters
	struct station_config stconf;
	memset(&stconf, 0, sizeof(stconf));
	strncpy((char *)stconf.ssid, szWifiSsid, sizeof(stconf.ssid));
	strncpy((char *)stconf.password, szWifiPassword, sizeof(stconf.password));
	stconf.ssid[sizeof(stconf.ssid)-1]='\0';
	stconf.password[sizeof(stconf.password)-1]='\0';

	//Initialize AP config parameters
	struct softap_config apconf;
	memset(&apconf, 0, sizeof(apconf));
	// SSID
	strncpy((char *)apconf.ssid, szWifiSsid, sizeof(apconf.ssid));
	apconf.ssid[sizeof(apconf.ssid)-1]='\0';
	// Password & encryption
	strncpy((char *)apconf.password, szWifiPassword, sizeof(apconf.password));
	apconf.password[sizeof(apconf.password)-1]='\0';
	if (strlen(szWifiPassword) >= 8) {
		apconf.authmode = AUTH_WPA_WPA2_PSK;
	} else {
		// if password <8 characters, don't use password.
		apconf.authmode = AUTH_WEP;
		memset(apconf.password, 0, sizeof(apconf.password));
	}
	apconf.max_connection = 255;
	apconf.beacon_interval = 100;

	wifi_set_opmode_current(nWifiMode);
	wifi_softap_set_config_current(&apconf);
	wifi_station_set_config_current(&stconf);

	struct ip_info ipinfo;
	memset(&ipinfo, 0, sizeof(ipinfo));
	ipinfo.ip.addr = aIP;
	ipinfo.netmask.addr = aNetmask;
	ipinfo.gw.addr = aGateway;

	if (nWifiMode == STATION_MODE) {
		if (bUseDhcp) {
			wifi_station_dhcpc_start();
		} else {
			wifi_station_dhcpc_stop();
			wifi_set_ip_info(STATION_IF, &ipinfo);
		}
	} else {
		// DHCP Server should be stopped to change IP information
		wifi_softap_dhcps_stop();
		wifi_set_ip_info(SOFTAP_IF, &ipinfo);

		uint32_t aFirstIp = (aIP & aNetmask) | 1;
		uint32_t aLastIp = (aIP & aNetmask) | (-1 & ~aNetmask);
		struct dhcps_lease leases;
		memset(&leases, 0, sizeof(leases));
		// Determine whether the range before or after our IP is bigger
		if (aIP - aFirstIp > aLastIp - aIP) {
			leases.start_ip.addr = aFirstIp;
			leases.end_ip.addr = aIP - 1;
		} else {
			leases.start_ip.addr = aIP + 1;
			leases.end_ip.addr = aLastIp;
		}
		wifi_softap_set_dhcps_lease(&leases);
		uint8_t offer_router = 0;
		wifi_softap_set_dhcps_offer_option(OFFER_ROUTER, &offer_router);
		wifi_softap_dhcps_start();
	}

#ifdef ENABLE_WS2812
	ws2812_init();
#else
	ws2801_init();
#endif
    //Wait for system to be done.
    system_init_done_cb(&system_is_done);
    //system_os_task()
}
