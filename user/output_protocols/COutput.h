#ifndef OUTPUT_PROTOCOLS_COUTPUT_H
#define OUTPUT_PROTOCOLS_COUTPUT_H
#include <stdint.h>

class COutput {
	public:
		COutput(unsigned int nLength);
		unsigned int getLength();
		virtual ~COutput();
		virtual void output(const uint8_t *pData);
	protected:
		unsigned int m_nLength;
};
#endif//OUTPUT_PROTOCOLS_COUTPUT_H
