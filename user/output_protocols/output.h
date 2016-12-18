#ifndef OUTPUT_PROTOCOLS_OUTPUT_H
#define OUTPUT_PROTOCOLS_OUTPUT_H
#include <stdint.h>
#include <stddef.h>
#include "config/config.h"

namespace Output {
	DEFINE_CONFIG(config);
	void init();
	void deinit();
	void output(const uint8_t *pData, size_t nLength);
	void partial(size_t nOffset, const uint8_t *pData, size_t nLength, bool bFlush);
	void try_push_frame();
	void limiter_timer_cb(void *);
}
#endif//OUTPUT_PROTOCOLS_OUTPUT_H
