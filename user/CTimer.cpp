#include "CTimer.h"
extern "C" {
#include <osapi.h>
}

CTimer::CTimer() {
	os_timer_disarm(&m_timer);
	m_bRunning = false;
	m_bRepeat = false;
}

CTimer::~CTimer() {
	os_timer_disarm(&m_timer);
}

void CTimer::onTrigger() {}

void CTimer::Stop() {
	os_timer_disarm(&m_timer);
	m_bRunning = false;
	m_bRepeat = false;
}

void CTimer::Start(uint32_t nMilliseconds, bool bRepeat) {
	if (m_bRunning)
		os_timer_disarm(&m_timer);
	os_timer_setfn(&m_timer, &CTimer::timer_cb, (void *)this);
	os_timer_arm(&m_timer, nMilliseconds, bRepeat);
	m_bRepeat = bRepeat;
	m_bRunning = true;
}

void CTimer::timer_cb(void *pTimer) {
	CTimer *pThis = (CTimer *)pTimer;
	if (pThis->m_bRunning) {
		if (!pThis->m_bRepeat)
			pThis->m_bRunning = false;
		pThis->onTrigger();
	}
}
