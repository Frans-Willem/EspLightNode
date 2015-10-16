#include "output.h"
#include "config/config.h"
#include "COutput.h"

enum OutputMode {
	Output_Dummy,
};
uint32_t nOutputMode;
uint32_t nOutputLength;

BEGIN_CONFIG(output,"Output")
CONFIG_SELECTSTART("mode", "Mode", &nOutputMode, Output_Dummy);
CONFIG_SELECTOPTION("None", Output_Dummy);
CONFIG_SELECTEND();
CONFIG_INT("length","Number of pixels",&nOutputLength, 1, 256, 150);
END_CONFIG();


COutput* pOutput = NULL;

void output_init() {
	switch (nOutputMode) {
		default:
			pOutput = new COutput(nOutputLength);
			break;
	}	
}

COutput* output_get() {
	return pOutput;
}
