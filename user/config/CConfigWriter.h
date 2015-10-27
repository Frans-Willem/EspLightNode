#ifndef CONFIG_CCONFIGWRITER_H
#define CONFIG_CCONFIGWRITER_H
#include <stdlib.h>
#include <stdint.h>

class CConfigWriter {
public:
	// nFirstSector is the sector where writing starts
	// nDirection is 1 or -1, depending on which direction should be written.
	// e.g. nFirstSector 200 and nDirection 1 would mean that sector 200, 201, 202, ... will be written
	// whereas nFirstSector 100 and nDirection -1 would mean 100, 99, 98, 97
	CConfigWriter(unsigned int nFirstSector, int nDirection);
	bool flush(bool bPad);
	void writeByte(const uint8_t nByte);
	void writeBytes(const uint8_t* pBytes, size_t nLength);
	void writeUInt(unsigned int nValue);
	void writeString(const char* szString);
	void writeFloat(float fValue);
private:
	unsigned int	m_nCurrentSector;	//Current sector
	int		m_nDirection;		//Which direction to continue after this sector is done
	unsigned int	m_nOffset;		//Offset in current sector
	uint8_t		m_bBuffer[64];
	size_t		m_nBufferFilled;
};
#endif//CONFIG_CCONFIGWRITER_H
