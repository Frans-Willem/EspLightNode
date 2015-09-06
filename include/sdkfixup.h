#ifndef __SDKFIXUP_H_
#define __SDKFIXUP_H_
#include <stddef.h>

// Include Espressif version first, then undefine, then include LibC version
#include <c_types.h>
#undef __packed__
#include <sys/cdefs.h>

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
#endif//__SDKFIXUP_H_
