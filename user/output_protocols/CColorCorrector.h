#ifndef OUTPUT_PROTOCOLS_CCOLORCORRECTOR_H
#define OUTPUT_PROTOCOLS_CCOLORCORRECTOR_H
#include "COutput.h"

class CColorCorrector : public COutput {
	public:
		CColorCorrector(COutput *pNext, bool bGamma, float fGamma, bool bLum2Duty);
		~CColorCorrector();
		static COutput* wrap(COutput *pNext, bool bGamma, float fGamma, bool bLum2Duty);
		virtual void output(const uint8_t *pData);
	private:
		void initLookup(bool bGamma, float fGamma, bool bLum2Duty);
		COutput *m_pNext;
		uint8_t *m_pBuffer;
		uint8_t m_lookup[256];
};
#endif//OUTPUT_PROTOCOLS_CCOLORCORRECTOR_H

