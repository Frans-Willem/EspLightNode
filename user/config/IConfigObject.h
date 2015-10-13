#ifndef CONFIG_ICONFIGOBJECT_H
#define CONFIG_ICONFIGOBJECT_H
#include <stdlib.h>

class IConfigObject {
	public:
		virtual ~IConfigObject() {};
		virtual IConfigObject* findChild(const char* szName) = 0;
		virtual bool getUint(unsigned int* pnValue) = 0;
		virtual bool getString(const char* szValue, size_t nLen) = 0;
};
#endif//CONFIG_ICONFIGOBJECT_H
