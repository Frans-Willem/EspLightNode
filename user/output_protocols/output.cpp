#include "output.h"
#include "config/config.h"
#include "COutput.h"
#include "CWS2801Output.h"

enum OutputMode {
	Output_Dummy,
	Output_WS2801
};
uint32_t nOutputMode;
uint32_t nOutputLength;

BEGIN_CONFIG(output,"Output")
CONFIG_SELECTSTART("mode", "Mode", &nOutputMode, Output_Dummy);
CONFIG_SELECTOPTION("None", Output_Dummy);
CONFIG_SELECTOPTION("WS2801 (Bit-bang)", Output_WS2801);
CONFIG_SELECTEND();
CONFIG_INT("length","Number of pixels",&nOutputLength, 1, 256, 150);
CONFIG_SUB(CWS2801Output::config);
END_CONFIG();


COutput* pOutput = NULL;

void output_init() {
	switch (nOutputMode) {
		case Output_WS2801:
			pOutput = new CWS2801Output(nOutputLength);
			break;
		default:
			pOutput = new COutput(nOutputLength);
			break;
	}	
}

COutput* output_get() {
	return pOutput;
}
