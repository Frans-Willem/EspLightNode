#ifndef OUTPUT_PROTOCOLS_CWS2801OUTPUT_H
#define OUTPUT_PROTOCOLS_CWS2801OUTPUT_H
#include "COutput.h"
#include "config/config.h"

class ISPIInterface;
class CWS2801Output : public COutput {
	public:
		CWS2801Output(unsigned int nLength, ISPIInterface *pSpi);
		~CWS2801Output();
		virtual void output(const uint8_t *pData);
	private:
		ISPIInterface *m_pSpi;
};
#endif//OUTPUT_PROTOCOLS_CWS2801OUTPUT_H
