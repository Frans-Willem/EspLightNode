#ifndef OUTPUT_PROTOCOLS_CSPIBITBANG_H
#define OUTPUT_PROTOCOLS_CSPIBITBANG_H
#include "ISPIInterface.h"
#include "config/config.h"

class CSPIBitbang : public ISPIInterface {
	public:
		CSPIBitbang();
		~CSPIBitbang();
		void output(const uint8_t *pData, size_t nDataLen);

		static DEFINE_CONFIG(config);
	private:
		static unsigned int g_nDataPin;
		static unsigned int g_nClockPin;
		unsigned int m_nDataBit;
		unsigned int m_nClockBit;
};
#endif//OUTPUT_PROTOCOLS_CSPIBITBANG_H
