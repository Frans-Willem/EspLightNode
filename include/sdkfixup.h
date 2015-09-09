#ifndef __SDKFIXUP_H_
#define __SDKFIXUP_H_
#include <stddef.h>

// Include Espressif version first, then undefine, then include LibC version
#define int8_t int8_t_espressif
#define int16_t int16_t_espressif
#define int32_t int32_t_espressif
#define uint8_t uint8_t_espressif
#define uint16_t uint16_t_espressif
#define uint32_t uint32_t_espressif
extern "C" {
#include <c_types.h>
}
#undef int8_t
#undef int16_t
#undef int32_t
#undef uint8_t
#undef uint16_t
#undef uint32_t
#undef __packed__

#include <sys/cdefs.h>
#include <stdint.h>

extern "C" {
#include <os_type.h>

	void *pvPortMalloc( size_t xWantedSize );
	void vPortFree( void *pv );
	void *pvPortZalloc(size_t size);

	void ets_sprintf(char *, const char *, ...);
	void* ets_memcpy(void *, const void*,size_t);
	void ets_intr_lock();
	void ets_intr_unlock();
	void ets_wdt_disable();
	void ets_delay_us(uint32_t);
	void ets_timer_disarm(os_timer_t *);
	void ets_timer_arm_new(os_timer_t *, uint32_t, bool, int);
	void ets_timer_setfn(os_timer_t *, os_timer_func_t *, void *);
}
#endif//__SDKFIXUP_H_
