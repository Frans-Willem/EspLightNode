#include "CConfigLoader.h"
#include "config/CConfigSection.h"
#include "config/format.h"
#include "config/CConfigReader.h"
#include "config/config.h"
#include "string.h"

void CConfigLoader::load() {
	//Allocate big classes on the heap instead of the stack.
	CConfigReader *pReader = new CConfigReader(CONFIG_START_SECTOR, CONFIG_SECTOR_DIRECTION);
	CConfigLoader loader;
	CConfigSection *pRootSection=new CConfigSection();
	const uint8_t pExpectedHeader[] = CONFIG_HEADER;
	uint8_t pHeader[sizeof(pExpectedHeader)];
	pReader->readBytes(pHeader, sizeof(pExpectedHeader));
	if (memcmp(pHeader, pExpectedHeader, sizeof(pExpectedHeader)) != 0 || !pRootSection->read(pReader)) {
		delete pRootSection;
		pRootSection = NULL;
	}
	delete pReader;

	loader.m_lSectionStack.push_back(pRootSection);
	config_run(&loader);

	if (pRootSection)
		delete pRootSection;
}

void CConfigLoader::beginModule(const char *szName, const char *szDescription) {
	CConfigSection *pSubSection;
	if (m_lSectionStack.empty() || m_lSectionStack.back() == NULL || !m_lSectionStack.back()->getSection(szName, &pSubSection))
		pSubSection = NULL;
	m_lSectionStack.push_back(pSubSection);
}

void CConfigLoader::endModule() {
	// Only pop if there will be at least one left,
	// We don't want to constantly have to check for emptiness.
	if (m_lSectionStack.size() > 1)
		m_lSectionStack.pop_back();
}

void CConfigLoader::optionBool(const char *szName, const char *szDescription, bool *pbValue, bool bDefault) {
	unsigned int nValue;
	if (!m_lSectionStack.empty() && m_lSectionStack.back() && m_lSectionStack.back()->getUInt(szName, nValue))
		*pbValue = (nValue != 0);
	else
		*pbValue = bDefault;
}

void CConfigLoader::optionString(const char *szName, const char *szDescription, char *szValue, size_t nSize, const char *szDefault) {
	if (m_lSectionStack.empty() || !m_lSectionStack.back() || !m_lSectionStack.back()->getString(szName, szValue, nSize)) {
		strncpy(szValue, szDefault, nSize);
		szValue[nSize-1]='\0';
	}
}

void CConfigLoader::optionInt(const char *szName, const char *szDescription, void *pValue, size_t nSize, uint32_t nMin, uint32_t nMax, uint32_t nDefault) {
	unsigned int nValue;
	if (m_lSectionStack.empty() || !m_lSectionStack.back() || !m_lSectionStack.back()->getUInt(szName, nValue))
		nValue = nDefault;
	if (nValue < nMin)
		nValue = nMin;
	if (nValue > nMax)
		nValue = nMax;
	switch (nSize) {
		case 1: (*(uint8_t*)pValue) = nValue; break;
		case 2: (*(uint16_t*)pValue) = nValue; break;
		case 4: (*(uint32_t*)pValue) = nValue; break;
	}
}
void CConfigLoader::optionSelectBegin(const char *szName, const char *szDescription, unsigned int* pnValue, unsigned int nDefault) {
	unsigned int nValue;
	if (m_lSectionStack.empty() || !m_lSectionStack.back() || !m_lSectionStack.back()->getUInt(szName, nValue))
		nValue = nDefault;
	*pnValue = nValue;

	m_pnSelectValue = pnValue;
	m_nDefaultSelectValue = nDefault;
	m_bValidSelectValue = false;
}
void CConfigLoader::optionSelectItem(const char *szName, unsigned int nValue) {
	if (nValue == *m_pnSelectValue)
		m_bValidSelectValue = true;
}
void CConfigLoader::optionSelectEnd() {
	if (!m_bValidSelectValue)
		*m_pnSelectValue = m_nDefaultSelectValue;
}
void CConfigLoader::optionIpAddress(const char *szName, const char *szDescription, uint32_t *pAddress, uint32_t nDefault) {
	if (m_lSectionStack.empty() || !m_lSectionStack.back() || !m_lSectionStack.back()->getUInt(szName, *pAddress))
		*pAddress = nDefault;
}
void CConfigLoader::optionFloat(const char *szName, const char *szDescription, float* pfValue, float fDefault) {
	if (m_lSectionStack.empty() || !m_lSectionStack.back() || !m_lSectionStack.back()->getFloat(szName, *pfValue))
		*pfValue = fDefault;
}

