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
#include "debug/CDebugServer.h"

namespace tpm2net {
	bool bEnabled;
	size_t nLastPacket;
	size_t nPreviousLength;

	BEGIN_CONFIG(config,"tpm2","TPM2.net");
		CONFIG_BOOLEAN("enabled","Enabled",&bEnabled, true);
	END_CONFIG();

	static void recv(void *arg, char *pusrdata, unsigned short length) {
		DEBUG("Packet");
		uint8_t *data =(uint8_t *)pusrdata; //pointer to espconn's returned data
		for (unsigned int i=0; i<16 && i <length; i++)
			DEBUG("%02X",data[i]);
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
						DEBUG("Package %d %d %u", packagenum, numpackages, nLastPacket, nPreviousLength);
						if (packagenum <= 1 || packagenum == nLastPacket + 1) {
							if (packagenum == 1)
								nPreviousLength = 0;
							Output::partial(nPreviousLength, &data[6], framelength, packagenum == numpackages);
							nPreviousLength += framelength;
							nLastPacket = packagenum;
						}
					}
				}
			}
		}
	}

	void init() {
		static struct espconn tpm2conn;
		static esp_udp tpm2udp;

		if (!bEnabled)
			return;

		tpm2conn.type = ESPCONN_UDP;
		tpm2conn.state = ESPCONN_NONE;
		tpm2conn.proto.udp = &tpm2udp;
		tpm2udp.local_port=0xFFE2;
		tpm2conn.reverse = NULL;
		espconn_regist_recvcb(&tpm2conn, recv);
		espconn_create(&tpm2conn);
	}

	void deinit() {

	}
}//namespace tpm2net
