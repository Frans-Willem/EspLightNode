#ifndef _WS2812_H
#define _WS2812_H

#define WSGPIO 0

#include "ets_sys.h"

void ICACHE_FLASH_ATTR ws2812_strip( uint8_t * buffer, uint16_t length );
void ICACHE_FLASH_ATTR ws2812_init();

#define GPIO_OUTPUT_SET(gpio_no, bit_value) gpio_output_set(bit_value<<gpio_no, ((~bit_value)&0x01)<<gpio_no, 1<<gpio_no,0)

#endif

