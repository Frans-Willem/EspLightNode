#ifndef ICONFIGRUNNER_H
#define ICONFIGRUNNER_H
#include <stddef.h>
#include <cstdint>
class IConfigRunner {
	public:
		virtual ~IConfigRunner();
		virtual void beginModule(const char *szName, const char *szDescription);
		virtual void endModule();
		virtual void optionBool(const char *szName, const char *szDescription, bool *pbValue, bool bDefault);
		virtual void optionString(const char *szName, const char *szDescription, char *szValue, size_t nSize, const char *szDefault);
		virtual void optionInt(const char *szName, const char *szDescription, void *pValue, size_t nSize, uint32_t nMin, uint32_t nMax, uint32_t nDefault);
		virtual void optionSelectBegin(const char *szName, const char *szDescription, unsigned int* pnValue, unsigned int nDefault);
		virtual void optionSelectItem(const char *szName, unsigned int nValue);
		virtual void optionSelectEnd();
};
#endif//ICONFIGRUNNER_H
