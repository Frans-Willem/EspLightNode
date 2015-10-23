#ifndef OUTPUT_PROTOCOLS_CWS2801HSPIOUTPUT_H
#define OUTPUT_PROTOCOLS_CWS2801HSPIOUTPUT_H
#include <sdkfixup.h>
extern "C" {
#include "osapi.h"
}
#include "COutput.h"
#include "config/config.h"

class CWS2801HSPIOutput : public COutput {
	public:
		CWS2801HSPIOutput(unsigned int nLength);
		~CWS2801HSPIOutput();
		virtual void output(const uint8_t *pData);
	private:
		static void timer(void *pThis);
		os_timer_t m_timer;
};
#endif//OUTPUT_PROTOCOLS_CWS2801HSPIOUTPUT_H

