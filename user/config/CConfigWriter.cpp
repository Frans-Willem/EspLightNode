#include <sdkfixup.h>
#include "config/CConfigWriter.h"
extern "C" {
#include <spi_flash.h>
}
#include <algorithm>
#include <string.h>

CConfigWriter::CConfigWriter(unsigned int nFirstSector, int nDirection) {
	m_nCurrentSector = nFirstSector;
	m_nDirection = nDirection;
	m_nOffset = 0;
	m_nBufferFilled = 0;
}

bool CConfigWriter::flush(bool bPad) {
	size_t nFlushed = 0;
	// Possibly pad to 4 bytes, as SPI write always wants at least that.
	if (bPad && m_nBufferFilled % sizeof(uint32_t) != 0) {
		size_t nPad = sizeof(uint32_t) - (m_nBufferFilled % sizeof(uint32_t));
		memset(&m_bBuffer[m_nBufferFilled], 0, nPad);
		m_nBufferFilled += nPad;
	}
	// Write out as much as possible
	while (m_nBufferFilled - nFlushed >= sizeof(uint32_t)) {
		size_t nWrite = std::min(SPI_FLASH_SEC_SIZE - m_nOffset, m_nBufferFilled - nFlushed);
		nWrite -= nWrite % sizeof(uint32_t);
		if (m_nOffset == 0)
			spi_flash_erase_sector(m_nCurrentSector);
		spi_flash_write((m_nCurrentSector * SPI_FLASH_SEC_SIZE) + m_nOffset, (uint32_t *)&m_bBuffer[nFlushed], nWrite);
		m_nOffset += nWrite;
		nFlushed += nWrite;
		if (m_nOffset >= SPI_FLASH_SEC_SIZE) {
			m_nOffset = 0;
			m_nCurrentSector += m_nDirection;
		}
	}
	// Move any remaining bits and pieces
	m_nBufferFilled -= nFlushed;
	memmove(m_bBuffer, &m_bBuffer[nFlushed], m_nBufferFilled);
	return (m_nBufferFilled == 0);
}

void CConfigWriter::writeByte(const uint8_t nByte) {
	if (m_nBufferFilled >= sizeof(m_bBuffer))
		flush(false);
	m_bBuffer[m_nBufferFilled] = nByte;
	m_nBufferFilled++;
}

void CConfigWriter::writeBytes(const uint8_t* pBytes, size_t nLength) {
	while (nLength) {
		if (m_nBufferFilled >= sizeof(m_bBuffer))
			flush(false);
		size_t nWrite = std::min(nLength, sizeof(m_bBuffer) - m_nBufferFilled);
		memcpy(&m_bBuffer[m_nBufferFilled], pBytes, nWrite);
		m_nBufferFilled += nWrite;
		nLength -= nWrite;
		pBytes = &pBytes[nWrite];
	}
}

void CConfigWriter::writeUInt(unsigned int nValue) {
	uint8_t nBit;
	do {
		nBit = nValue & 0x7F;
		nValue >>= 7;
		if (nValue)
			nBit |= 0x80;
		writeByte(nBit);
	} while(nBit & 0x80);
}

void CConfigWriter::writeString(const char* szString) {
	size_t nLen = strlen(szString);
	writeUInt(nLen);
	writeBytes((const uint8_t *)szString, nLen);
}
