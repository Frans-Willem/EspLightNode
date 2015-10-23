#include "output.h"
#include "config/config.h"
#include "COutput.h"
#include "CWS2801Output.h"
#include "CSPIBitbang.h"
#include "CSPIHardware.h"

enum OutputMode {
	Output_Dummy,
	Output_WS2801_BB,
	Output_WS2801_HSPI
};
uint32_t nOutputMode;
uint32_t nOutputLength;

BEGIN_CONFIG(output,"Output")
CONFIG_SELECTSTART("mode", "Mode", &nOutputMode, Output_Dummy);
CONFIG_SELECTOPTION("None", Output_Dummy);
CONFIG_SELECTOPTION("WS2801 (SPI Bitbang)", Output_WS2801_BB);
CONFIG_SELECTOPTION("WS2801 (HSPI)", Output_WS2801_HSPI);
CONFIG_SELECTEND();
CONFIG_INT("length","Number of pixels",&nOutputLength, 1, 256, 150);
CONFIG_SUB(CSPIBitbang::config);
CONFIG_SUB(CSPIHardware::config);
END_CONFIG();


COutput* pOutput = NULL;

void output_init() {
	switch (nOutputMode) {
		case Output_WS2801_BB:
			pOutput = new CWS2801Output(nOutputLength, new CSPIBitbang());
			break;
		case Output_WS2801_HSPI:
			pOutput = new CWS2801Output(nOutputLength, new CSPIHardware());
			break;
		default:
			pOutput = new COutput(nOutputLength);
			break;
	}	
}

COutput* output_get() {
	return pOutput;
}
