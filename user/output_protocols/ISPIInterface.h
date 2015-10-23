#ifndef OUTPUT_PROTOCOLS_ISPIINTERFACE_H
#define OUTPUT_PROTOCOLS_ISPIINTERFACE_H
#include <stddef.h>
#include <stdint.h>

class ISPIInterface {
public:
	virtual ~ISPIInterface();
	virtual void output(const uint8_t *pData, size_t nDataLen);
};
#endif//OUTPUT_PROTOCOLS_ISPIINTERFACE_H

