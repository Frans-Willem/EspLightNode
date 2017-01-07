#ifndef OUTPUT_PROTOCOLS_CLIMITER_H
#define OUTPUT_PROTOCOLS_CLIMITER_H
#include "COutput.h"
#include "../CTimer.h"

class CLimiter : CTimer, public COutput {
	public:
	   	CLimiter(COutput *pNext, unsigned int nPeriod);
		~CLimiter();
		static COutput* wrap(COutput *pNext, unsigned int nMaxFramerate);

		virtual void output(const uint8_t *pData);
	private:
		virtual void onTrigger();

		enum State {
			Idle, //Idle state
			Waiting, //Frame was recently sent, timer is running.
			Queued //Timer is still running, new frame already queued.
		};

		COutput *m_pNext;
		unsigned int m_nPeriod;
		uint8_t *m_pBuffer;
		State m_state;

};
#endif//OUTPUT_PROTOCOLS_CLIMITER_H

