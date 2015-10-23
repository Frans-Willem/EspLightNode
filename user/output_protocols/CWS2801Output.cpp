/*
 * user_ws2801.c
 *
 *  Created on: Nov 18, 2014
 *      Author: frans-willem
 */
#include "CWS2801Output.h"
#include "ISPIInterface.h"
#include <debug/CDebugServer.h>



CWS2801Output::CWS2801Output(unsigned int nLength, ISPIInterface *pSpi) : COutput(nLength) {
	m_pSpi = pSpi;
}

CWS2801Output::~CWS2801Output() {
	delete m_pSpi;
}

void CWS2801Output::output(const uint8_t *pData) {
	m_pSpi->output(pData, m_nLength * 3);
}
