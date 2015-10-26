/*
 * tpm2net.c
 *
 *  Created on: Nov 18, 2014
 *      Author: frans-willem
 */
#include <sdkfixup.h>
extern "C" {
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
}
#include <config/config.h>
#include "output_protocols/output.h"
#include "output_protocols/COutput.h"

bool tpm2net_enabled;
uint16_t framebuffer_len = 0;
uint8_t framebuffer[1536]; //max 512 rgb pixels

BEGIN_CONFIG(tpm2net,"tpm2","TPM2.net");
	CONFIG_BOOLEAN("enabled","Enabled",&tpm2net_enabled, 1);
END_CONFIG();

static void ICACHE_FLASH_ATTR tpm2net_recv(void *arg, char *pusrdata, unsigned short length) {
    uint8_t *data =(uint8_t *)pusrdata; //pointer to espconn's returned data
    if (data && length >= 6 && data[0]==0x9C) { // header identifier (packet start)
        uint8_t blocktype = data[1]; // block type
        uint16_t framelength = ((uint16_t)data[2] << 8) | (uint16_t)data[3]; // frame length
        uint8_t packagenum = data[4]; // packet number 0-255 (0x00 = no split)
        uint8_t numpackages = data[5]; // total packets 1-255
        if (blocktype == 0xDA) { // data command ...
            if (length >= framelength + 7 && data[6+framelength]==0x36) { // header end (packet stop)
                if (numpackages == 0x01) { // no frame split found
					Output::output(&data[6], framelength);
                } else { //frame split is found
                    os_memcpy (&framebuffer[framebuffer_len], &data[6], framelength);
                    framebuffer_len += framelength;
                    if (packagenum == numpackages) { // all packets found 
						Output::output(framebuffer, framebuffer_len);
                        framebuffer_len = 0;
                    }
                }
            }
        }
    }
}

void tpm2net_init() {
	static struct espconn tpm2conn;
	static esp_udp tpm2udp;

	tpm2conn.type = ESPCONN_UDP;
	tpm2conn.state = ESPCONN_NONE;
	tpm2conn.proto.udp = &tpm2udp;
	tpm2udp.local_port=0xFFE2;
	tpm2conn.reverse = NULL;
	espconn_regist_recvcb(&tpm2conn, tpm2net_recv);
	espconn_create(&tpm2conn);
}
