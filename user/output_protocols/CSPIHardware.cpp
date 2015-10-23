#include <sdkfixup.h>
#include "CSPIHardware.h"
extern "C" {
#include <gpio.h>
#include <spi_register.h>
#include <ets_sys.h>
}
#include <string.h>
#include <algorithm>
#pragma GCC diagnostic ignored "-Wparentheses"
#include <debug/CDebugServer.h>

unsigned int CSPIHardware::g_nClockDiv = 0;
bool CSPIHardware::g_bUseInterrupts = true;

BEGIN_CONFIG(CSPIHardware::config,"HSPI configuration");
CONFIG_SELECTSTART("clockdiv","Speed",&g_nClockDiv,0);
CONFIG_SELECTOPTION("40mhz", 0); //80/(0+1)
CONFIG_SELECTOPTION("20mhz", 1); //80/(1+1)
CONFIG_SELECTOPTION("14mhz", 2); //80/(2+1) =
CONFIG_SELECTOPTION("10mhz", 3); //80/(3+1)
CONFIG_SELECTOPTION("5mhz", 7);
CONFIG_SELECTOPTION("2.5mhz", 15);
CONFIG_SELECTEND();
CONFIG_BOOLEAN("use_int", "Interrupt driven", &g_bUseInterrupts, true);
END_CONFIG();

CSPIHardware::CSPIHardware() {
	m_nBytesLeft = 0;
	m_bTransmissionDone = true;
	m_pWords = m_pCurWords = 0;

	// TODO: Check this, what is this value before, and what exactly should be cleared?
	WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105); //clear bit 9?
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); // Clock
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); // Data
	// Only use MOSI, disable all other options
	WRITE_PERI_REG(SPI_USER(1), SPI_USR_MOSI);
	//Clock
	WRITE_PERI_REG(SPI_CLOCK(1),
			((g_nClockDiv&SPI_CLKDIV_PRE)<<SPI_CLKDIV_PRE_S)| // Clock pre-divider
			((1&SPI_CLKCNT_N)<<SPI_CLKCNT_N_S)| //Total clock length
			((1&SPI_CLKCNT_H)<<SPI_CLKCNT_H_S)|
			((0&SPI_CLKCNT_L)<<SPI_CLKCNT_L_S));

	ETS_SPI_INTR_ATTACH((void*)&CSPIHardware::int_handler, (void*)this);
	if (g_bUseInterrupts)
		ETS_SPI_INTR_ENABLE();
	else
		ETS_INTR_DISABLE(ETS_SPI_INUM);
}

CSPIHardware::~CSPIHardware() {

}

void CSPIHardware::output(const uint8_t *pData, size_t nDataLen) {
	//How many 32-bit words to store this?
	size_t nWordsLen = ((nDataLen + 3) / 4);
	uint32_t *pWords = new uint32_t[nWordsLen];
	memset(pWords, 0, sizeof(uint32_t)*nWordsLen);
	//Convert
	for (unsigned int i=0; i<nDataLen; i++)
		pWords[i/4] |= pData[i] << ((i%4) * 8);
	if (g_bUseInterrupts) {
		ETS_INTR_DISABLE(ETS_SPI_INUM);
		if (!m_bTransmissionDone) {
			//Previous transmission still busy
			ETS_INTR_ENABLE(ETS_SPI_INUM);
			return;
		}
		if (m_pWords)
			delete[] m_pWords;
		m_pWords = m_pCurWords = pWords;
		m_nBytesLeft = nDataLen;
		m_bTransmissionDone = false;
		ETS_INTR_ENABLE(ETS_SPI_INUM);
		//Start transmission here
		int_handler_transmit_done();
	} else {
		size_t nCurWord = 0;
		while (nCurWord < nWordsLen) {
			size_t nBytes = std::min<size_t>(nDataLen - (nCurWord * 4), 16 * 4);
			size_t nBits = (nBytes*8)-1;

			while (READ_PERI_REG(SPI_CMD(1)) & SPI_USR); // Wait for transmission to end
			for (unsigned int reg = SPI_W0(1); reg <= SPI_W15(1) && nCurWord < nWordsLen; reg+=4, nCurWord++)
				WRITE_PERI_REG(reg, pWords[nCurWord]);

			WRITE_PERI_REG(SPI_USER1(1), nBits << SPI_USR_MOSI_BITLEN_S);
			SET_PERI_REG_MASK(SPI_CMD(1), SPI_USR);
		}
		delete[] pWords;
	}
}

void __attribute__((section(".nospi"))) CSPIHardware::int_handler(void *pThis) {
	if (READ_PERI_REG(0x3FF00020) & BIT7) {
		// HSPI
		unsigned int regvalue = READ_PERI_REG(SPI_SLAVE(1));
		CLEAR_PERI_REG_MASK(SPI_SLAVE(1), (SPI_TRANS_DONE|SPI_SLV_WR_STA_DONE|SPI_SLV_RD_STA_DONE | SPI_SLV_WR_BUF_DONE|SPI_SLV_RD_BUF_DONE) & regvalue);

		if (regvalue & SPI_TRANS_DONE) {
			((CSPIHardware *)pThis)->int_handler_transmit_done();
		}
	}
	if (READ_PERI_REG(0x3ff00020) & BIT4) {
		// SPI
		CLEAR_PERI_REG_MASK(SPI_SLAVE(0), 0x3FF);
	}
}

void __attribute__((section(".nospi"))) CSPIHardware::int_handler_transmit_done() {
	if (m_nBytesLeft) {
		size_t nBytes = std::min<size_t>(m_nBytesLeft, 16 * 4);
		size_t nBits = (nBytes*8)-1;
		size_t nWords = (nBytes+3)/4;

		for (unsigned int reg = SPI_W0(1); reg < SPI_W0(1) + (4*nWords); reg+=4)
			WRITE_PERI_REG(reg, *(m_pCurWords++));
		// Set number of bits
		WRITE_PERI_REG(SPI_USER1(1), nBits << SPI_USR_MOSI_BITLEN_S);
		// Mark as done
		m_nBytesLeft -= nBytes;
		// Start transmission
		SET_PERI_REG_MASK(SPI_CMD(1), SPI_USR);
	} else {
		m_bTransmissionDone = true;
	}
}
