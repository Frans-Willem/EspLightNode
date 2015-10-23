#include <sdkfixup.h>
#include "CWS2801HSPIOutput.h"
#include "debug/CDebugServer.h"
#pragma GCC diagnostic ignored "-Wparentheses"
extern "C" {
	#include <gpio.h>
    #include <spi_register.h>
#include <ets_sys.h>
}

void spi_int_handler(void*);
CWS2801HSPIOutput::CWS2801HSPIOutput(unsigned int nLength) : COutput(nLength) {

	os_timer_setfn(&m_timer, (os_timer_func_t *)timer, this);
	os_timer_arm(&m_timer, 500, 1);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	gpio_output_set(0, BIT0, BIT0, 0);
	
	//bit9 of PERIPHS_IO_MUX should be cleared when HSPI clock doesn't equal CPU clock
	//bit8 of PERIPHS_IO_MUX should be cleared when SPI clock doesn't equal CPU clock
	WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105); //clear bit 9?
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); // Clock
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); // Data
	/*
	SET_PERI_REG_MASK(SPI_USER(1), SPI_USR_MOSI);
	CLEAR_PERI_REG_MASK(SPI_USER(1), SPI_CS_SETUP|SPI_CS_HOLD|SPI_FLASH_MODE);
	*/
	WRITE_PERI_REG(SPI_USER(1), SPI_USR_MOSI);
	// Clear quad or dual mode
	CLEAR_PERI_REG_MASK(SPI_USER(1), SPI_QIO_MODE|SPI_DIO_MODE|SPI_DOUT_MODE|SPI_QOUT_MODE);
	//Clock
	WRITE_PERI_REG(SPI_CLOCK(1),
			((7&SPI_CLKDIV_PRE)<<SPI_CLKDIV_PRE_S)| // Clock pre-divider
			((1&SPI_CLKCNT_N)<<SPI_CLKCNT_N_S)| //Total clock length
			((1&SPI_CLKCNT_H)<<SPI_CLKCNT_H_S)|
			((0&SPI_CLKCNT_L)<<SPI_CLKCNT_L_S));

	/*
	ets_isr_attach(ETS_SPI_INUM+1,(void *)&spi_int_handler, (void*)123);
	ets_isr_mask(ETS_SPI_INUM+1);
	*/
	ETS_SPI_INTR_ATTACH((void*)spi_int_handler, NULL);
	ETS_SPI_INTR_ENABLE();
}
CWS2801HSPIOutput::~CWS2801HSPIOutput() {

}
void CWS2801HSPIOutput::output(const uint8_t *pData) {

}

volatile unsigned int nLeft = 0;

void CWS2801HSPIOutput::timer(void *pThis) {
	// Wait for transmission end
	while (READ_PERI_REG(SPI_CMD(1)) & SPI_USR);

	//Set mosi len
	CLEAR_PERI_REG_MASK(SPI_USER1(1), SPI_USR_MOSI_BITLEN << SPI_USR_MOSI_BITLEN_S);
	SET_PERI_REG_MASK(SPI_USER1(1), (63 & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S);
	for (unsigned int reg = SPI_W0(1); reg <= SPI_W15(1); reg+=4)
		WRITE_PERI_REG(reg, 0xF0F0F0F0);

	GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, BIT0);
	GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, BIT0);
	nLeft = 3;
	SET_PERI_REG_MASK(SPI_CMD(1), SPI_USR); //transmission start


	GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, BIT0);
	GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, BIT0);

}

void __attribute__((section(".nospi"))) spi_int_handler(void*) {
	if (READ_PERI_REG(0x3FF00020) & BIT7) {
		// HSPI
		unsigned int regvalue = READ_PERI_REG(SPI_SLAVE(1));
		CLEAR_PERI_REG_MASK(SPI_SLAVE(1), (SPI_TRANS_DONE|SPI_SLV_WR_STA_DONE|SPI_SLV_RD_STA_DONE | SPI_SLV_WR_BUF_DONE|SPI_SLV_RD_BUF_DONE) & regvalue);

		if (regvalue & SPI_TRANS_DONE) {
			if (nLeft) {
				nLeft--;
				for (unsigned int reg = SPI_W0(1); reg <= SPI_W15(1); reg+=4)
					WRITE_PERI_REG(reg, nLeft);
				SET_PERI_REG_MASK(SPI_CMD(1), SPI_USR); //transmission start
			} else {
				GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, BIT0);
				GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, BIT0);
			}
		}
	}
	if (READ_PERI_REG(0x3ff00020) & BIT4) {
		// SPI
		CLEAR_PERI_REG_MASK(SPI_SLAVE(0), 0x3FF);
	}
}

