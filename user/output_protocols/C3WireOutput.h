#ifndef OUTPUT_PROTOCOLS_CI2SHARDWARE_H
#define OUTPUT_PROTOCOLS_CI2SHARDWARE_H
#include <sdkfixup.h>
#include "COutput.h"
extern "C" {
#include <eagle_soc.h>
#include <osapi.h>
#include <gpio.h>
#include <slc_register.h>
#include <ets_sys.h>
#include <slc_slv.h>
}

class I3WireEncoder;
class C3WireOutput : public COutput {
	public:
		C3WireOutput(unsigned int nLength, unsigned int nBck, unsigned int nDiv, I3WireEncoder *pEncoder);
		~C3WireOutput();
		virtual void output(const uint8_t *pData);
	private:
		I3WireEncoder *m_pEncoder;
		sdio_queue m_qBuffer;
		uint32_t *m_pBuffer;
		size_t m_nBufferLength;
		sdio_queue m_qZeroes;
		uint32_t m_pZeroes[1];
};
#endif//OUTPUT_PROTOCOLS_CI2SHARDWARE_H
