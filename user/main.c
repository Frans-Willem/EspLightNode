/*
 * user_main.c
 *
 *  Created on: Nov 15, 2014
 *      Author: frans-willem
 */


#include <input_protocols/tpm2net.h>
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"

#include "output_protocols/ws2801.h"
#include "input_protocols/artnet.h"
#include "input_protocols/tpm2net.h"
#include "config/httpd.h"
#include "config/config.h"

static volatile os_timer_t client_timer;
static void ICACHE_FLASH_ATTR wait_for_ip(uint8 flag) {
    LOCAL struct ip_info ipconfig;
    LOCAL int status;
    os_timer_disarm(&client_timer);

    status = wifi_station_get_connect_status();
    if (status == STATION_GOT_IP) {
        wifi_get_ip_info(STATION_IF, &ipconfig);
        if( ipconfig.ip.addr != 0) {
        	//Start UDP server
#ifdef ENABLE_TPM2
        	tpm2net_init();
#endif
#ifdef ENABLE_ARTNET
        	artnet_init();
#endif
        	httpd_init();
        } else {
            os_timer_setfn(&client_timer, (os_timer_func_t *)wait_for_ip, NULL);
            os_timer_arm(&client_timer, 100, 0);
        }
    } else if (status == STATION_CONNECTING) {
        os_timer_setfn(&client_timer, (os_timer_func_t *)wait_for_ip, NULL);
        os_timer_arm(&client_timer, 100, 0);
    } else { //STATION_NO_AP_FOUND||STATION_CONNECT_FAIL||STATION_WRONG_PASSWORD
    	//Connection failed, somehow :(
        system_restart();
        //Bring up SOFTAP here
    }
}

static void ICACHE_FLASH_ATTR system_is_done(void){
	//Bringing up WLAN
	wifi_station_connect();
	//Wait for connection
	os_timer_disarm(&client_timer);
	os_timer_setfn(&client_timer, (os_timer_func_t *)wait_for_ip, NULL);
	os_timer_arm(&client_timer, 100, 0);
}

//Init function
void ICACHE_FLASH_ATTR
user_init()
{
	config_load();

	struct station_config stconf;
	memset(&stconf, 0, sizeof(stconf));
	strcpy((char *)stconf.ssid,WIFI_SSID);
	strcpy((char *)stconf.password,WIFI_PASSWORD);

	//Don't attempt to call wifi_disconnect or wifi_connect, it'll bite crash on you.
	wifi_station_set_auto_connect(0);
	if (wifi_get_opmode() != STATION_MODE) {
		wifi_set_opmode(STATION_MODE); //station
		os_delay_us(500);
		system_restart();
	}

	wifi_station_set_config(&stconf);
	wifi_station_set_auto_connect(1);

    ws2801_init();

    //Wait for system to be done.
    system_init_done_cb(&system_is_done);
    //system_os_task()
}
