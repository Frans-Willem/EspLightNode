#include "CConfigSection.h"
#include "config/CConfigReader.h"
#include "config/format.h"
#include "string.h"

CConfigSection::CConfigSection() {
}

CConfigSection::~CConfigSection() {
	for (auto i : m_mSubSections)
		delete i.second;
}

bool CConfigSection::read(CConfigReader *pReader) {
	while (true) {
		switch (pReader->readUInt()) {
			case ConfigSectionEnd:{
				return true;
					      }
			case ConfigSectionStart:{
				std::string szName(pReader->readString());
				CConfigSection *pSubSection = new CConfigSection();
				if (!pSubSection->read(pReader)) {
					delete pSubSection;
					return false;
				}
				auto found = m_mSubSections.find(szName);
				if (found != m_mSubSections.end()) {
					delete found->second;
					found->second = pSubSection;
				} else {
					m_mSubSections[szName] = pSubSection;
				}
				break;
						}
			case ConfigString:{
				std::string szName(pReader->readString());
				std::string szValue(pReader->readString());
				m_mStrings[szName]=szValue;
				break;
					  }
			case ConfigInteger:{
				std::string szName(pReader->readString());
				m_mIntegers[szName] = pReader->readUInt();
				break;
					   }
			case ConfigFloat:{
				std::string szName(pReader->readString());
				m_mFloats[szName] = pReader->readFloat();
				break;
					   }
		}
	}
}

bool CConfigSection::getUInt(const std::string szKey, unsigned int &nValue) {
	const auto found = m_mIntegers.find(szKey);
	if (found == m_mIntegers.end())
		return false;
	nValue = found->second;
	return true;
}

bool CConfigSection::getString(const std::string szKey, char *szString, size_t nSize) {
	const auto found = m_mStrings.find(szKey);
	if (found == m_mStrings.end())
		return false;
	strncpy(szString, found->second.c_str(), nSize-1);
	szString[nSize-1]='\0';
	return true;
}

bool CConfigSection::getString(const std::string szKey, std::string& szValue) {
	const auto found = m_mStrings.find(szKey);
	if (found == m_mStrings.end())
		return false;
	szValue = found->second;
	return true;
}

bool CConfigSection::getSection(const std::string szKey, CConfigSection** ppSection) {
	const auto found = m_mSubSections.find(szKey);
	if (found == m_mSubSections.end())
		return false;
	*ppSection = found->second;
	return true;
}

bool CConfigSection::getFloat(const std::string szKey, float& fValue) {
	const auto found = m_mFloats.find(szKey);
	if (found == m_mFloats.end())
		return false;
	fValue = found->second;
	return true;
}
