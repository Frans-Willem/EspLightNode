#include <sdkfixup.h>
#include "CConfigReader.h"
#include <string.h>
extern "C" {
#include <spi_flash.h>
}

CConfigReader::CConfigReader(unsigned int nFirstSector, int nDirection) {
	m_nCurrentSector = nFirstSector;
	m_nDirection = nDirection;
	m_nBufferSize = m_nBufferOffset = 0;
	m_nOffset = 0;
}

uint8_t CConfigReader::readByte() {
	if (m_nBufferOffset >= m_nBufferSize)
		fillBuffer();
	return m_bBuffer[m_nBufferOffset++];
}

void CConfigReader::readBytes(uint8_t* pBytes, size_t nLength) {
	while (nLength) {
		if (m_nBufferOffset >= m_nBufferSize)
			fillBuffer();
		size_t nAvailable = std::min(nLength, m_nBufferSize-m_nBufferOffset);
		memcpy(pBytes, &m_bBuffer[m_nBufferOffset], nAvailable);
		m_nBufferOffset += nAvailable;
		nLength -= nAvailable;
		pBytes = &pBytes[nAvailable];
	}
}

void CConfigReader::fillBuffer() {
	// Discard all already-read bytes, but ensure that the number of kept bytes is divisible by uint32_t
	ptrdiff_t nDiscard = m_nBufferOffset;
	if ((m_nBufferSize - nDiscard) % sizeof(uint32_t) != 0)
		nDiscard -= sizeof(uint32_t) - ((m_nBufferSize - nDiscard) % sizeof(uint32_t));
	if (nDiscard >= 0)
		memmove(m_bBuffer, &m_bBuffer[nDiscard], m_nBufferSize - nDiscard);
	else
		memmove(&m_bBuffer[-nDiscard], &m_bBuffer, m_nBufferSize);
	m_nBufferSize -= nDiscard; 
	m_nBufferOffset -= nDiscard;
	
	// Possibly skip to next sector
	if (m_nOffset >= SPI_FLASH_SEC_SIZE) {
		m_nOffset = 0;
		m_nCurrentSector += m_nDirection;
	}
	// How much can we read?
	size_t nSpace = std::min<size_t>(sizeof(m_bBuffer)-m_nBufferSize, SPI_FLASH_SEC_SIZE - m_nOffset);
	nSpace -= nSpace % sizeof(uint32_t);
	if (!nSpace)
		return;

	spi_flash_read(m_nCurrentSector, (uint32_t *)&m_bBuffer[m_nBufferSize], nSpace);
	m_nBufferSize += nSpace;
}
