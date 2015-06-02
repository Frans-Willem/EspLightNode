#include "ws2812.h"
#include "osapi.h"

void ICACHE_FLASH_ATTR SEND_WS_0()
{
	uint8_t time;
	time = 4; while(time--) WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + GPIO_ID_PIN(WSGPIO), 1 );
	time = 9; while(time--) WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + GPIO_ID_PIN(WSGPIO), 0 );
}

void ICACHE_FLASH_ATTR SEND_WS_1()
{
	uint8_t time; 
	time = 8; while(time--) WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + GPIO_ID_PIN(WSGPIO), 1 );
	time = 6; while(time--) WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + GPIO_ID_PIN(WSGPIO), 0 );
}

void ICACHE_FLASH_ATTR ws2812_strip( uint8_t * buffer, uint16_t length )
{
	uint16_t i;
	GPIO_OUTPUT_SET(GPIO_ID_PIN(WSGPIO), 0);

	os_intr_lock();
	for( i = 0; i < length; i++ )
	{
		uint8_t mask = 0x80;
		uint8_t byte = buffer[i];
		while (mask) 
		{
			( byte & mask ) ? SEND_WS_1() : SEND_WS_0();
			mask >>= 1;
        	}
	}
	os_intr_unlock();
}

void ICACHE_FLASH_ATTR ws2812_init()
{
	ets_wdt_disable();
	char outbuffer[] = { 0x00, 0x00, 0x00 };
	ws2812_out( outbuffer, sizeof(outbuffer) );
}

