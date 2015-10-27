#ifndef CONFIG_CCONFIGSECTION_H
#define CONFIG_CCONFIGSECTION_H
#include <string>
#include <map>

class CConfigReader;
class CConfigSection {
public:
	CConfigSection();
	~CConfigSection();
	bool read(CConfigReader *pRead);
	bool getUInt(const std::string szKey, unsigned int &nValue);
	bool getString(const std::string szKey, char *szString, size_t nSize);
	bool getString(const std::string szKey, std::string &szValue);
	bool getSection(const std::string szKey, CConfigSection** ppSection);
	bool getFloat(const std::string szKey, float& fValue);
private:

	std::map<std::string, unsigned int> m_mIntegers;
	std::map<std::string, std::string> m_mStrings;
	std::map<std::string, CConfigSection*> m_mSubSections;
	std::map<std::string, float> m_mFloats;
};
#endif//CONFIG_CCONFIGSECTION_H
