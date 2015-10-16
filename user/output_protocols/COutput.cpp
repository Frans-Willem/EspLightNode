#include "COutput.h"

COutput::COutput(unsigned int nLength) : m_nLength(nLength) {
}
COutput::~COutput() {
}
unsigned int COutput::getLength() {
	return m_nLength;
}
void COutput::output(const uint8_t *pData) {
}
