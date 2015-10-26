#include "output.h"
#include "config/config.h"
#include "COutput.h"
#include "CWS2801Output.h"
#include "CSPIBitbang.h"
#include "CSPIHardware.h"
#include "C3WireOutput.h"
#include "C3WireEncoder.h"
#include <algorithm>

namespace Output {
	enum OutputMode {
		Output_Dummy,
		Output_WS2801_BB,
		Output_WS2801_HSPI,
		Output_WS281X_800,
		Output_WS281X_400
	};
	// Config
	uint32_t nOutputMode;
	uint32_t nOutputLength;
	bool bLum2Duty;
	// Globals
	COutput* pOutput = NULL;
	uint8_t *pCorrectionTable;
	uint8_t *pLastOutput;

	BEGIN_CONFIG(config,"output","Output")
	CONFIG_SELECTSTART("mode", "Mode", &nOutputMode, Output_Dummy);
	CONFIG_SELECTOPTION("None", Output_Dummy);
	CONFIG_SELECTOPTION("WS2801 (SPI Bitbang)", Output_WS2801_BB);
	CONFIG_SELECTOPTION("WS2801 (HSPI)", Output_WS2801_HSPI);
	CONFIG_SELECTOPTION("WS2811/WS2812 @ 800khz", Output_WS281X_800);
	CONFIG_SELECTOPTION("WS2811/WS2812 @ 400khz", Output_WS281X_400);
	CONFIG_SELECTEND();
	CONFIG_INT("length","Number of channels",&nOutputLength, 1, 512, 450);
	CONFIG_BOOLEAN("lum2duty","Luminance correction", &bLum2Duty, false);
	CONFIG_SUB(CSPIBitbang::config);
	CONFIG_SUB(CSPIHardware::config);
	END_CONFIG();



	void init() {
		pLastOutput = new uint8_t[nOutputLength];
		switch (nOutputMode) {
			case Output_WS2801_BB:
				pOutput = new CWS2801Output(nOutputLength, new CSPIBitbang());
				break;
			case Output_WS2801_HSPI:
				pOutput = new CWS2801Output(nOutputLength, new CSPIHardware());
				break;
			case Output_WS281X_800:
				pOutput = new C3WireOutput(nOutputLength, 10, 5, new C3WireEncoder<4,4,1,2>());
				break;
			case Output_WS281X_400:
				pOutput = new C3WireOutput(nOutputLength, 10, 10, new C3WireEncoder<4,4,1,2>());
				break;
			default:
				pOutput = new COutput(nOutputLength);
				break;
		}
		if (bLum2Duty) {
			pCorrectionTable=new uint8_t[256];
			for (unsigned int i=0; i<256; i++) {
				float fValue = ((float)i) / 255.0f;
				//See:
				// https://ledshield.wordpress.com/2012/11/13/led-brightness-to-your-eye-gamma-correction-no/
				if (fValue > 0.07999591993063804f) {
					fValue = ((fValue+0.16f)/1.16f);
					fValue *= fValue * fValue;
				} else {
					fValue /= 9.033f;
				}

				long nValue = std::round(fValue*255.0f);
				if (nValue<0) pCorrectionTable[i] = 0;
				else if (nValue>255) pCorrectionTable[i] = 255;
				else pCorrectionTable[i] = nValue;
			}
		}
	}

	void deinit() {
		delete pOutput;
		delete[] pLastOutput;
		if (pCorrectionTable)
			delete[] pCorrectionTable;
	}

	void output(const uint8_t *pData, size_t nLength) {
		nLength = std::min(nLength, nOutputLength);
		if (pCorrectionTable) {
			for (size_t i = 0; i<nLength; i++)
				pLastOutput[i] = pCorrectionTable[pData[i]];
		} else
			memcpy(pLastOutput, pData, nLength);
		pOutput->output(pLastOutput);
	}	
}//namespace Output
