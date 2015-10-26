#include <sdkfixup.h>
extern "C" {
#include <ets_sys.h>
#include <user_interface.h>
#include <osapi.h>
}
#include "wifisetup.h"
#include "config/config.h"
#include <string.h>

char szWifiDefaultSsid[32];
char szDefaultHostname[64];

unsigned int nWifiMode;
char szWifiSsid[32];
char szWifiPassword[64];
bool bUseDhcp;
char szHostname[64];
uint32_t aIP;
uint32_t aNetmask;
uint32_t aGateway;

BEGIN_CONFIG(wifi, "wifi", "WiFi");
CONFIG_SELECTSTART("mode","Operation mode", &nWifiMode, SOFTAP_MODE);
CONFIG_SELECTOPTION("Access point", SOFTAP_MODE);
CONFIG_SELECTOPTION("Client", STATION_MODE);
CONFIG_SELECTEND();
CONFIG_STRING("ssid", "SSID", szWifiSsid, sizeof(szWifiSsid)-1, szWifiDefaultSsid);
CONFIG_STRING("password","Password", szWifiPassword, sizeof(szWifiPassword)-1,"");
END_CONFIG();
BEGIN_CONFIG(network, "net", "Network");
CONFIG_STRING("hostname", "Hostname", szHostname, sizeof(szHostname)-1, szDefaultHostname);
CONFIG_BOOLEAN("dhcp","Automatic configuration (DHCP)", &bUseDhcp, true);
CONFIG_IP("ip","IP Address", &aIP, 192, 168, 1, 1);
CONFIG_IP("netmask","Netmask", &aNetmask, 255, 255, 255, 0);
CONFIG_IP("gateway","Gateway", &aGateway, 192, 168, 1, 1);
END_CONFIG();

void wifi_setup_defaults() {
	os_sprintf(szWifiDefaultSsid,"EspLightNode%X", system_get_chip_id());
	os_sprintf(szDefaultHostname, "EspLightNode%X", system_get_chip_id());
}

void wifi_init() {
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
		wifi_station_dhcpc_stop();
		wifi_station_set_hostname(szHostname);
		if (bUseDhcp) {
			wifi_station_dhcpc_start();
		} else {
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
}
