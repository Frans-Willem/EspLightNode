#include <sdkfixup.h>
#include "helpers.h"
extern "C" {
#include <osapi.h>
}

float eln_atof(const char *nptr) {
	float fRetval = 0.0f;
	float fMulti = 1.0f;
	float fSign = 1.0f;
	if (*nptr == '-')
		fSign = -1.0f;
	bool bAfterComma = false;
	for (; *nptr != '\0'; nptr++) {
		if (*nptr >= '0' && *nptr <= '9') {
			if (bAfterComma) {
				fMulti /= 10.0f;
				fRetval += fMulti * (float)(*nptr - '0');
			} else {
				fRetval *= 10.0f;
				fRetval += (float)(*nptr - '0');
			}
		} else if (*nptr == '.' || *nptr == ',') {
			bAfterComma = true;
		}
	}
	fRetval *= fSign;
	return fRetval;
}

void eln_ftoa(float fValue, char *ptr, size_t max) {
	if (max == 0) return;
	if (max < 3) {
		*ptr = '\0';
		return;
	}
	if (fValue < 0.0f) {
		*ptr = '-';
		eln_ftoa(0.0f-fValue,&ptr[1],max - 1);
		return;
	}
	int nInt = (int)fValue;
	int nWritten = ets_snprintf(ptr,max-2,"%d",nInt);
	ptr[nWritten]='.';
	nWritten++;
	do {
		fValue = (fValue - nInt) * 10.0f;
		nInt = (int)fValue;
		ptr[nWritten] = '0' + nInt;
		nWritten++;
	} while (fValue > (float)nInt && nWritten + 1 < max);
	ptr[nWritten] = '\0';
}

int ets_snprintf(char *str, size_t size, const char *format, ...) {
	va_list args;
	__builtin_va_start(args, format);
	int retval = ets_vsnprintf(str, size, format, args);
	__builtin_va_end(args);
	return retval;
}

