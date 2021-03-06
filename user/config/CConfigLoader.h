#ifndef CONFIG_CCONFIGLOADER_H
#define CONFIG_CCONFIGLOADER_H
#include "config/IConfigRunner.h"
#include <list>

class CConfigSection;
class CConfigLoader : IConfigRunner {
public:
	static void load();
private:
	void beginModule(const char *szName, const char *szDescription);
	void endModule();
	void optionBool(const char *szName, const char *szDescription, bool *pbValue, bool pbDefault);
	void optionString(const char *szName, const char *szDescription,  char *szValue, size_t nSize, const char *szDefault);
	void optionInt(const char *szName, const char *szDescription, void *pValue, size_t nSize, uint32_t nMin, uint32_t nMax, uint32_t nDefault);
	void optionSelectBegin(const char *szName, const char *szDescription, unsigned int* pnValue, unsigned int nDefault);
	void optionSelectItem(const char *szName, unsigned int nValue);
	void optionSelectEnd();
	void optionIpAddress(const char *szName, const char *szDescription, uint32_t *pAddress, uint32_t nDefault);
	void optionFloat(const char *szName, const char *szDescription, float* pfValue, float fDefault);

	std::list<CConfigSection*> m_lSectionStack;	
	unsigned int* m_pnSelectValue;
	unsigned int m_nDefaultSelectValue;
	bool m_bValidSelectValue;
};
#endif//CONFIG_CCONFIGLOADER_H
