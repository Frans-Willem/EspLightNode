#ifndef OUTPUT_PROTOCOLS_CWS2801OUTPUT_H
#define OUTPUT_PROTOCOLS_CWS2801OUTPUT_H
#include "COutput.h"
#include "config/config.h"

class CWS2801Output : public COutput {
	public:
		CWS2801Output(unsigned int nLength);
		~CWS2801Output();
		virtual void output(const uint8_t *pData);

		static DEFINE_CONFIG(config);
	private:
		static unsigned int g_nDataPin;
		static unsigned int g_nClockPin;
		unsigned int m_nDataBit;
		unsigned int m_nClockBit;
};
#endif//OUTPUT_PROTOCOLS_CWS2801OUTPUT_H
