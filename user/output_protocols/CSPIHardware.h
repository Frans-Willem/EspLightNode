#ifndef OUTPUT_PROTOCOLS_CSPIHARDWARE_H
#define OUTPUT_PROTOCOLS_CSPIHARDWARE_H
#include "ISPIInterface.h"
#include "config/config.h"

class CSPIHardware : public ISPIInterface {
	public:
		CSPIHardware();
		~CSPIHardware();
		void output(const uint8_t *pData, size_t nDataLen);

		static DEFINE_CONFIG(config);
	private:
		static void int_handler(void *pThis);
		void int_handler_transmit_done();
		static unsigned int g_nClockDiv;
		static bool g_bUseInterrupts;

		uint32_t *m_pWords;
		uint32_t *m_pCurWords;
		size_t m_nBytesLeft;
		bool m_bTransmissionDone;
};
#endif//OUTPUT_PROTOCOLS_CSPIHARDWARE_H
