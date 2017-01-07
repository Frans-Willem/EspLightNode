#include "CColorCorrector.h"
#include <algorithm>

CColorCorrector::CColorCorrector(COutput *pNext, bool bGamma, float fGamma, bool bLum2Duty)
	: COutput(pNext->getLength())
{
	m_pBuffer = new uint8_t[m_nLength];
	m_pNext = pNext;
	initLookup(bGamma, fGamma, bLum2Duty);
}

CColorCorrector::~CColorCorrector() {
	delete[] m_pBuffer;
	delete m_pNext;
}

COutput* CColorCorrector::wrap(COutput *pNext, bool bGamma, float fGamma, bool bLum2Duty) {
	if (!bGamma && !bLum2Duty)
		return pNext;
	return new CColorCorrector(pNext, bGamma, fGamma, bLum2Duty);
}

void CColorCorrector::output(const uint8_t *pData) {
	for (unsigned int i = 0; i < m_nLength; i++) {
		m_pBuffer[i] = m_lookup[pData[i]];
	}
	m_pNext->output(m_pBuffer);
}

void CColorCorrector::initLookup(bool bGamma, float fGamma, bool bLum2Duty) {
	for (unsigned int i=0; i<256; i++) {
		float fValue = ((float)i)/255.0f;
		if (bGamma)
			fValue = pow(fValue, fGamma);
		if (bLum2Duty) {
			//See:
			// https://ledshield.wordpress.com/2012/11/13/led-brightness-to-your-eye-gamma-correction-no/
			if (fValue > 0.07999591993063804f) {
				fValue = ((fValue+0.16f)/1.16f);
				fValue *= fValue * fValue;
			} else {
				fValue /= 9.033f;
			}
		}

		long nValue = std::round(fValue*255.0f);
		if (nValue<0) m_lookup[i] = 0;
		else if (nValue>255) m_lookup[i] = 255;
		else m_lookup[i] = nValue;
	}
}
