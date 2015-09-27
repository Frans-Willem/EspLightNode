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

	std::list<CConfigSection*> m_lSectionStack;	
};
#endif//CONFIG_CCONFIGLOADER_H
