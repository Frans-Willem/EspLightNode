#include "CLimiter.h"
#include <string.h>

CLimiter::CLimiter(COutput *pNext, unsigned int nPeriod)
	: COutput(pNext->getLength())
{
	m_pNext = pNext;
	m_nPeriod = nPeriod;
	m_pBuffer = new uint8_t[m_nLength];
	m_state = State::Idle;
}

CLimiter::~CLimiter() {
	delete m_pNext;
	delete[] m_pBuffer;
}

COutput* CLimiter::wrap(COutput *pNext, unsigned int nMaxFramerate) {
	if (nMaxFramerate == 0)
		return pNext;
	return new CLimiter(pNext, 1000/nMaxFramerate);
}

void CLimiter::output(const uint8_t *pData) {
	switch (m_state) {
		case State::Idle: // On idle, just output, and start timeout for next frame.
			m_state = State::Waiting;
			Start(m_nPeriod, false);
			m_pNext->output(pData);
			break;
		case State::Waiting: // When still waiting, queue up.
			m_state = State::Queued;
			memcpy(m_pBuffer, pData, m_nLength * sizeof(uint8_t));
			break;
		case State::Queued: // When already queued, discard previous.
			m_state = State::Queued;
			memcpy(m_pBuffer, pData, m_nLength * sizeof(uint8_t));
			break;
	}
}

void CLimiter::onTrigger() {
	switch (m_state) {
		case State::Idle:
		case State::Waiting:
			m_state = State::Idle;
			break;
		case State::Queued:
			m_state = State::Waiting;
			Start(m_nPeriod, false);
			m_pNext->output(m_pBuffer);
			break;
	}
}
