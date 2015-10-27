#ifndef CONFIG_CCONFIGREADER_H
#define CONFIG_CCONFIGREADER_H
#include <stdlib.h>
#include <stdint.h>
#include <string>

class CConfigReader {
public:
	CConfigReader(unsigned int nFirstSector, int nDirection);
	uint8_t readByte();
	void readBytes(uint8_t* pBytes, size_t nLength);
	void skip(size_t nLength);
	unsigned int readUInt();
	std::string readString();
	float readFloat();
private:
	unsigned int	m_nCurrentSector;
	int		m_nDirection;
	unsigned int	m_nOffset;
	uint8_t		m_bBuffer[64];
	size_t		m_nBufferSize;
	size_t		m_nBufferOffset;

	void fillBuffer();
};
#endif//CONFIG_CCONFIGREADER_H

