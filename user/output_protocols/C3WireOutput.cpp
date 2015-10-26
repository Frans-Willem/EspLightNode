#pragma GCC diagnostic ignored "-Wparentheses"
#include <sdkfixup.h>
#include "C3WireOutput.h"
extern "C" {
#include <eagle_soc.h>
#include <osapi.h>
#include <gpio.h>
#include <slc_register.h>
#include <i2s_register.h>
#include <slc_slv.h>
#undef PIN_FUNC_SELECT
#include <pin_mux_register.h>
}
#include "C3WireEncoder.h"

// TODO: We should probably move these defines to a header somewhere.
#ifndef i2c_bbpll
#define i2c_bbpll                                 0x67
#define i2c_bbpll_en_audio_clock_out            4
#define i2c_bbpll_en_audio_clock_out_msb        7
#define i2c_bbpll_en_audio_clock_out_lsb        7
#define i2c_bbpll_hostid                           4

#define i2c_writeReg_Mask(block, host_id, reg_add, Msb, Lsb, indata) \
    rom_i2c_writeReg_Mask(block, host_id, reg_add, Msb, Lsb, indata)

#define i2c_readReg_Mask(block, host_id, reg_add, Msb, Lsb) \
    rom_i2c_readReg_Mask_(block, host_id, reg_add, Msb, Lsb)

#define i2c_writeReg_Mask_def(block, reg_add, indata) \
    i2c_writeReg_Mask(block, block##_hostid, reg_add, reg_add##_msb, reg_add##_lsb, indata)

#define i2c_readReg_Mask_def(block, reg_add) \
    i2c_readReg_Mask(block, block##_hostid, reg_add, reg_add##_msb, reg_add##_lsb)
#endif

extern "C" void rom_i2c_writeReg_Mask(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,uint32_t);

C3WireOutput::C3WireOutput(
		unsigned int nLength,
		unsigned int nBck,
		unsigned int nDiv,
		I3WireEncoder *pEncoder)
	: COutput(nLength)
{
	// Final clock frequency is:
	// 160 / MAX(2, nBck) / MAX(2, nDiv)
	//
	// Example:
	// For the WS2811 800khz clock,
	// one bit should take 1.25usec or 1250nanosec
	//   1sec/1250nanosec = 800khz
	// Assuming we need 4 bits to encode each bit, we'd need a clock of
	//   4 * 800khz = 3.2mhz
	// So we need to find 160 / nBck / nDiv = 3.2
	//   160 / 3.2 = 50
	// So we need nBck * nDiv = 50, where nBck>=2 and nDiv>=2
	//   We should use 5 and 10, or 2 and 25, etc
	m_pEncoder = pEncoder;
	m_nBufferLength = m_pEncoder->getMaxLength(m_nLength);
	m_pBuffer = new uint32_t[m_nBufferLength];

	memset(&m_qBuffer, 0, sizeof(m_qBuffer));
	memset(&m_qZeroes, 0, sizeof(m_qZeroes));
	memset(m_pZeroes, 0, sizeof(m_pZeroes));

	// DMA buffer structure.
	// Buffer points to actual data, zeroes points to zeroes
	// Both buffer and zeroes point to zeroes as "next buffer"
	m_qBuffer.owner = m_qZeroes.owner = 1;
	m_qBuffer.eof = m_qZeroes.eof = 1;
	m_qBuffer.sub_sof = m_qZeroes.sub_sof = 0;
	m_qBuffer.datalen = m_qBuffer.blocksize = m_nBufferLength * sizeof(uint32_t);
	m_qZeroes.datalen = m_qZeroes.blocksize = sizeof(m_pZeroes);
	m_qBuffer.buf_ptr = (uint32_t)m_pBuffer;
	m_qZeroes.buf_ptr = (uint32_t)m_pZeroes;
	m_qBuffer.unused = m_qZeroes.unused = 0;
	m_qBuffer.next_link_ptr = m_qZeroes.next_link_ptr = (uint32_t)&m_qZeroes;

	// Pin func select
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_I2SO_DATA);
	// We don't really care about anything other than data,
	// But you might want to un-comment these while debugging.
	// PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_I2SO_WS);
	// PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_I2SO_BCK);

	//Reset DMA
	SET_PERI_REG_MASK(SLC_CONF0, SLC_RXLINK_RST);
	CLEAR_PERI_REG_MASK(SLC_CONF0, SLC_RXLINK_RST);

	//Clear DMA int flags
	SET_PERI_REG_MASK(SLC_INT_CLR, 0xFFFFFFFF);
	CLEAR_PERI_REG_MASK(SLC_INT_CLR, 0xFFFFFFFF);

	//Enable and configure DMA
	CLEAR_PERI_REG_MASK(SLC_CONF0, (SLC_MODE << SLC_MODE_S));
	SET_PERI_REG_MASK(SLC_CONF0, (1<<SLC_MODE_S));
	SET_PERI_REG_MASK(SLC_RX_DSCR_CONF, SLC_INFOR_NO_REPLACE|SLC_TOKEN_NO_REPLACE);
	CLEAR_PERI_REG_MASK(SLC_RX_DSCR_CONF, SLC_RX_FILL_EN|SLC_RX_EOF_MODE|SLC_RX_FILL_MODE);

	CLEAR_PERI_REG_MASK(SLC_RX_LINK, SLC_RXLINK_DESCADDR_MASK);
	SET_PERI_REG_MASK(SLC_RX_LINK, ((uint32_t)&m_qZeroes) & SLC_RXLINK_DESCADDR_MASK);

	SET_PERI_REG_MASK(SLC_RX_LINK, SLC_RXLINK_START);

	//Clock provider?
	i2c_writeReg_Mask_def(i2c_bbpll, i2c_bbpll_en_audio_clock_out, 1);

	// Reset I2S subsystem
	CLEAR_PERI_REG_MASK(I2SCONF, I2S_I2S_RESET_MASK);
	SET_PERI_REG_MASK(I2SCONF, I2S_I2S_RESET_MASK);
	CLEAR_PERI_REG_MASK(I2SCONF, I2S_I2S_RESET_MASK);

	CLEAR_PERI_REG_MASK(I2S_FIFO_CONF, I2S_I2S_DSCR_EN|(I2S_I2S_RX_FIFO_MOD<<I2S_I2S_RX_FIFO_MOD_S)|(I2S_I2S_TX_FIFO_MOD<<I2S_I2S_TX_FIFO_MOD_S));
	SET_PERI_REG_MASK(I2S_FIFO_CONF, I2S_I2S_DSCR_EN);

	//Trans master & rece slave?

	CLEAR_PERI_REG_MASK(I2SCONF, I2S_TRANS_SLAVE_MOD|
						(I2S_BITS_MOD<<I2S_BITS_MOD_S)|
						(I2S_BCK_DIV_NUM <<I2S_BCK_DIV_NUM_S)|
						(I2S_CLKM_DIV_NUM<<I2S_CLKM_DIV_NUM_S));
	SET_PERI_REG_MASK(I2SCONF, I2S_RIGHT_FIRST|I2S_MSB_RIGHT|I2S_RECE_SLAVE_MOD|
						I2S_RECE_MSB_SHIFT|I2S_TRANS_MSB_SHIFT|
						(((nBck)&I2S_BCK_DIV_NUM )<<I2S_BCK_DIV_NUM_S)|
						(((nDiv)&I2S_CLKM_DIV_NUM)<<I2S_CLKM_DIV_NUM_S));

	//No idea if ints are needed...
	//clear int
	SET_PERI_REG_MASK(I2SINT_CLR, I2S_I2S_RX_WFULL_INT_CLR|I2S_I2S_PUT_DATA_INT_CLR|I2S_I2S_TAKE_DATA_INT_CLR);
	CLEAR_PERI_REG_MASK(I2SINT_CLR, I2S_I2S_RX_WFULL_INT_CLR|I2S_I2S_PUT_DATA_INT_CLR|I2S_I2S_TAKE_DATA_INT_CLR);
	//enable int
	SET_PERI_REG_MASK(I2SINT_ENA,  I2S_I2S_RX_REMPTY_INT_ENA|I2S_I2S_RX_TAKE_DATA_INT_ENA);


	//Start transmission
	SET_PERI_REG_MASK(I2SCONF,I2S_I2S_TX_START);
}

C3WireOutput::~C3WireOutput() {
	delete[] m_pBuffer;
	delete m_pEncoder;
}

void C3WireOutput::output(const uint8_t *pData) {
	// Stop DMA
	// Reset descriptor address
	SET_PERI_REG_MASK(SLC_RX_LINK, SLC_RXLINK_STOP);
	// TODO: Ideally this should be moved closer to the rest of the register access
	// But that will sometimes fail.
	// I suspect we have to wait for the SLC to actually be stopped before messing with the registers
	m_pEncoder->encode(pData, m_nLength, m_pBuffer);
	// Ideally I'd like to adjust the buffer length,
	// but that will creep out the SLC when it's not properly stopped.
	// So assume that the rest of the buffer is 0 anyway.
	// m_qBuffer.datalen = m_qBuffer.blocksize = nOutputLength * 4;

	// Adjust SLC (DMA) to point to just encoded buffer
	CLEAR_PERI_REG_MASK(SLC_RX_LINK,SLC_RXLINK_DESCADDR_MASK);
	SET_PERI_REG_MASK(SLC_RX_LINK, ((uint32)&m_qBuffer) & SLC_RXLINK_DESCADDR_MASK);

	// Reset FIFO, toggle pin, don't leave high
	SET_PERI_REG_MASK(I2SCONF, I2S_I2S_TX_FIFO_RESET);
	CLEAR_PERI_REG_MASK(I2SCONF, I2S_I2S_TX_FIFO_RESET);

	// Start DMA
	SET_PERI_REG_MASK(SLC_RX_LINK, SLC_RXLINK_START);
}
