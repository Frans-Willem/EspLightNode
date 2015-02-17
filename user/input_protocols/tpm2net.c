/*
 * tpm2net.c
 *
 *  Created on: Nov 18, 2014
 *      Author: frans-willem
 */
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"

#include "../output_protocols/ws2801.h"

uint16_t framebuffer_len = 0;
unsigned char framebuffer[1536]; //max 512 rgb pixels

static void ICACHE_FLASH_ATTR tpm2net_recv(void *arg, char *pusrdata, unsigned short length) {
    unsigned char *data =(unsigned char *)pusrdata; //pointer to espconn's returned data
    if (data && length >= 6 && data[0]==0x9C) { // header identifier (packet start)
        uint8_t blocktype = data[1]; // block type
        uint16_t framelength = ((uint16_t)data[2] << 8) | (uint16_t)data[3]; // frame length
        uint8_t packagenum = data[4]; // packet number 0-255 (0x00 = no split)
        uint8_t numpackages = data[5]; // total packets 1-255
        if (blocktype == 0xDA) { // data command ...
            if (length >= framelength + 7 && data[6+framelength]==0x36) { // header end (packet stop)
                if (packagenum == 0x01 && numpackages == 0x01) { // no frame split found
                    unsigned char *frame = &data[6]; // pointer 'frame' to espconn's data (start of data)
                    ws2801_strip(frame, framelength); // send data to strip
                } else { //frame split is found
                    os_memcpy (&framebuffer[framebuffer_len], &data[6], framelength);
                    framebuffer_len += framelength;
                    if (packagenum == numpackages) { // all packets found 
                        unsigned char *frame = &framebuffer[0]; // pointer 'frame' framebuffer
                        ws2801_strip(frame, framebuffer_len); // send data to strip
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
