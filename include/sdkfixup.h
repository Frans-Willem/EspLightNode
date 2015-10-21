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
#include <stdarg.h>

extern "C" {
#include <os_type.h>

	void *pvPortMalloc( size_t xWantedSize );
	void vPortFree( void *pv );
	void *pvPortZalloc(size_t size);

	typedef __builtin_va_list va_list;
	int ets_sprintf(char *, const char *, ...);
	int ets_vsprintf(char *str, const char *format, va_list ap);
	int ets_vsnprintf(char *str, size_t size, const char *format, va_list ap);
	void* ets_memcpy(void *, const void*,size_t);
	void ets_intr_lock();
	void ets_intr_unlock();
	void ets_wdt_disable();
	void ets_delay_us(uint32_t);
	void ets_timer_disarm(os_timer_t *);
	void ets_timer_arm_new(os_timer_t *, uint32_t, bool, int);
	void ets_timer_setfn(os_timer_t *, os_timer_func_t *, void *);
	void ets_isr_attach(int intr, void *handler, void *arg);
	void ets_isr_mask(unsigned intr);
	void ets_isr_unmask(unsigned intr);
}

extern "C" {
#include <ip_addr.h>
#include <espconn.h>
}
#ifndef ESPCONN_MAXNUM
#define ESPCONN_MAXNUM -7
#endif
#endif//__SDKFIXUP_H_
