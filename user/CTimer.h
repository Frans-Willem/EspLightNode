#ifndef CTIMER_H
#define CTIMER_H
#include <sdkfixup.h>

class CTimer {
	public:
		CTimer();
		virtual ~CTimer();
		virtual void onTrigger();
		void Stop();
		void Start(uint32_t nMilliseconds, bool bRepeat);
	private:
		os_timer_t m_timer;
		bool m_bRunning;
		bool m_bRepeat;

		static void timer_cb(void *pTimer);
};
#endif//CTIMER_H
