/*
 * user_ws2801.c
 *
 *  Created on: Nov 18, 2014
 *      Author: frans-willem
 */
#include <sdkfixup.h>
extern "C" {
	#include <c_types.h>
	#include <eagle_soc.h>
	#include <gpio.h>
}
#include "CWS2801Output.h"
#include "debug/CDebugServer.h"

#define BIT_CLK		BIT0
#define BIT_DATA	BIT2

struct GpioInfo {
	const char *szName;
	unsigned int nBit;
	unsigned int nMuxReg;
	unsigned int nMuxVal;
};

GpioInfo gpioInfo[] = {
	{"GPIO0/D3/SPICS2/CLK OUT", BIT0, PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0},
	{"GPIO1/D10/U0TXD/SPI CS1/CLK RTC", BIT1, PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1},
	{"GPIO2/D4/I2SO WS/U1TXD/U0TXD", BIT2, PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2},
	{"GPIO3/D9/U0RXD/I2SO DATA/CLK XTAL", BIT3, PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3},
	{"GPIO4/D2/CLK XTAL", BIT4, PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4},
	{"GPIO5/D1/CLK RTC", BIT5, PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5},
	{"GPIO9/SD_DATA2/SPIHD/HSPIHD", BIT9, PERIPHS_IO_MUX_SD_DATA2_U, FUNC_GPIO9},
	{"GPIO10/SD_DATA3/SPIWP/HSPIWP", BIT10, PERIPHS_IO_MUX_SD_DATA3_U, FUNC_GPIO10},
	{"GPIO12/D6/MTDI/I2SI DATA/HSPI MISO/U0DTR", BIT12, PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12},
	{"GPIO13/D7/MTCK/I2SI BCK/HSPI MOSI/U0CTS", BIT13, PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13},
	{"GPIO14/D5/MTMS/I2SI WS/HSPI CLK/U0DSR", BIT14, PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14},
	{"GPIO15/D1/MTDO/I2SO BCK/HSPI CS/U0RTS", BIT15, PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15}
};

unsigned int CWS2801Output::g_nDataPin = 0;
unsigned int CWS2801Output::g_nClockPin = 0;

BEGIN_CONFIG(CWS2801Output::config, "WS2801 configuration");
CONFIG_SELECTSTART("data","Data pin", &g_nDataPin, 2);
for (unsigned int i=0; i<sizeof(gpioInfo)/sizeof(gpioInfo[0]); i++)
	CONFIG_SELECTOPTION(gpioInfo[i].szName, i);
CONFIG_SELECTEND();
CONFIG_SELECTSTART("clock","Clock pin", &g_nClockPin, 0);
for (unsigned int i=0; i<sizeof(gpioInfo)/sizeof(gpioInfo[0]); i++)
	CONFIG_SELECTOPTION(gpioInfo[i].szName, i);
CONFIG_SELECTEND();
END_CONFIG();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
CWS2801Output::CWS2801Output(unsigned int nLength) : COutput(nLength) {
	/*
	PIN_FUNC_SELECT(gpioInfo[g_nDataPin].nMuxReg, gpioInfo[g_nDataPin].nMuxVal);
	PIN_FUNC_SELECT(gpioInfo[g_nClockPin].nMuxReg, gpioInfo[g_nClockPin].nMuxVal);
	*/
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

	m_nDataBit = gpioInfo[g_nDataPin].nBit;
	m_nClockBit = gpioInfo[g_nClockPin].nBit;
	gpio_output_set(0, m_nDataBit|m_nClockBit, m_nDataBit|m_nClockBit, 0);

}
#pragma GCC diagnostic pop

CWS2801Output::~CWS2801Output() {

}

void CWS2801Output::output(const uint8_t *pData) {
	unsigned int nTotalLength = m_nLength * 3;
	for (unsigned int i=0; i<nTotalLength; i++) {
		for (uint8_t bitmask=0x80; bitmask!=0; bitmask>>=1) {
			if (pData[i] & bitmask) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, m_nDataBit);
			else GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, m_nDataBit);
			GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, m_nClockBit);
			GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, m_nClockBit);
		}
	}
	GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, m_nDataBit);
}
