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
	bool bGamma;
	float fGamma;
	// Globals
	COutput* pOutput = NULL;
	uint8_t *pCorrectionTable;
	uint8_t *pNextFrame;
	uint8_t *pCurrentFrame;
	// Global related to framerate limiter
	static os_timer_t timerLimiter;
	bool bFrameReady = false; // pCurrentFrame should be sent out.
	bool bFrameShouldWait = false;
	uint32_t nLimiterFreq;
	uint32_t nLimiterPeriod; // Calculated from above

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
	CONFIG_BOOLEAN("gamma","Gamma correction", &bGamma, false);
	CONFIG_FLOAT("gammaValue","Gamma", &fGamma, 1.5f);
	CONFIG_INT("frameratecap","Maximum framerate", &nLimiterFreq, 0, 1000, 30);
	CONFIG_SUB(CSPIBitbang::config);
	CONFIG_SUB(CSPIHardware::config);
	END_CONFIG();



	void init() {
		pCurrentFrame = new uint8_t[nOutputLength];
		pNextFrame = new uint8_t[nOutputLength];
		nLimiterPeriod = (nLimiterFreq > 0) ? (1000/nLimiterFreq):0;
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
		if (bLum2Duty || bGamma) {
			pCorrectionTable=new uint8_t[256];
			for (unsigned int i=0; i<256; i++) {
				float fValue = ((float)i) / 255.0f;
				if (bGamma)
					fValue = pow(fValue, fGamma);
				if (bLum2Duty) {
					//See:
					// https://ledshield.wordpress.com/2012/11/13/led-brightness-to-your-eye-gamma-correction-no/
					if (fValue > 0.07999591993063804f) {
						fValue = ((fValue+0.16f)/1.16f);
						fValue *= fValue * fValue;
					} else {
						fValue /= 9.033f;
					}
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
		delete[] pCurrentFrame;
		delete[] pNextFrame;
		if (pCorrectionTable)
			delete[] pCorrectionTable;
	}

	void output(const uint8_t *pData, size_t nLength) {
		partial(0,pData,nLength,true);
	}

	void partial(size_t nOffset, const uint8_t *pData, size_t nLength, bool bFlush) {
		if (nOffset < nOutputLength) {
			if (nLength + nOffset > nOutputLength)
				nLength= nOutputLength-nOffset;
			uint8_t *pTarget = &pNextFrame[nOffset];
			if (pCorrectionTable) {
				for (size_t i = 0; i < nLength; i++)
					pTarget[i] = pCorrectionTable[pData[i]];
			} else {
				memcpy(pTarget, pData, nLength * sizeof(uint8_t));
			}

		}
		if (bFlush) {
			memcpy(pCurrentFrame, pNextFrame, nOutputLength * sizeof(uint8_t));
			bFrameReady = true;
			try_push_frame();
		}
	}

	// Try to push pCurrentFrame to output as soon as possible.
	// Will possibly wait for the limiter.
	void try_push_frame() {
		// Ignore if the limiter is active and a frame was just sent, or if no new frame is ready.
		if ((nLimiterPeriod > 0 && bFrameShouldWait) || !bFrameReady) {
			return;
		}
		bFrameReady = false;
		bFrameShouldWait = (nLimiterPeriod > 0);
		pOutput->output(pCurrentFrame);
		if (nLimiterPeriod > 0) {
			os_timer_disarm(&timerLimiter);
			os_timer_setfn(&timerLimiter, (os_timer_func_t *)&limiter_timer_cb, NULL);
			os_timer_arm(&timerLimiter, nLimiterPeriod, 0);
		}
	}

	void limiter_timer_cb(void *) {
		os_timer_disarm(&timerLimiter);
		bFrameShouldWait = false;
		try_push_frame();
	}
}//namespace Output
