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
#include "wifisetup.h"


void start_services() {
	CHttpServer *pServer = new CHttpServer(80);
	
	config_init(pServer);
	tpm2net_init();
	artnet_init();
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
	if (wifi_get_opmode() == STATION_MODE) {
		wifi_station_connect();
		//Wait for connection
		os_timer_disarm(&client_timer);
		os_timer_setfn(&client_timer, (os_timer_func_t *)wait_for_ip, NULL);
		os_timer_arm(&client_timer, 100, 0);
	} else if (wifi_get_opmode() == SOFTAP_MODE) {
		start_services();
	}
}

//Init function
extern "C" void ICACHE_FLASH_ATTR
user_init()
{
	wifi_setup_defaults();
	config_load();
	wifi_init();

#ifdef ENABLE_WS2812
	ws2812_init();
#else
	ws2801_init();
#endif
    //Wait for system to be done.
    system_init_done_cb(&system_is_done);
    //system_os_task()
}
