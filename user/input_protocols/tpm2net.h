/*
 * tpm2net.h
 *
 *  Created on: Nov 18, 2014
 *      Author: frans-willem
 */

#ifndef INPUT_PROTOCOLS_TPM2NET_H_
#define INPUT_PROTOCOLS_TPM2NET_H_
#include "config/config.h"

namespace tpm2net {
	DEFINE_CONFIG(config);
	void init();
	void deinit();
}
#endif /* INPUT_PROTOCOLS_TPM2NET_H_ */
