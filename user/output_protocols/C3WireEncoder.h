#ifndef C3WIREENCODER_H
#define C3WIREENCODER_H
#include <sdkfixup.h>
#include <stdint.h>
#include <algorithm>
#include <debug/CDebugServer.h>

class I3WireEncoder {
	public:
		virtual ~I3WireEncoder() {};
		virtual size_t getMaxLength(size_t nInputLength) = 0;
		virtual size_t encode(const uint8_t *pInput, size_t nInputLength, uint32_t *pOutput) = 0;
};

template<unsigned int len0, unsigned int len1, unsigned int num0, unsigned int num1>
class C3WireEncoder : public I3WireEncoder {
	public:
		size_t getMaxLength(size_t nInputLength) {
			unsigned int nBits = std::max(len0,len1) * 8 * nInputLength;
			return (nBits + 31)/32;
		}

		size_t encode(const uint8_t *pInput, size_t nInputLength, uint32_t *pOutput) {
			memset(pOutput, 0, getMaxLength(nInputLength)*sizeof(uint32_t));
			size_t nBitsLeft = 32;
			size_t nEncoded = 0;
			for (size_t i=0; i<nInputLength; i++) {
				uint8_t nInput = pInput[i];
				for (uint8_t bit = 0x80; bit!=0; bit >>= 1) {
					uint32_t nBitValue;
					size_t nBitLength; 
					if (nInput & bit) {
						nBitLength = len1;
						nBitValue = ((1 << num1)-1) << (len1-num1);
					} else {
						nBitLength = len0;
						nBitValue = ((1 << num0)-1) << (len0-num0);
					}
					if (nBitLength < nBitsLeft) {
						nBitsLeft -= nBitLength;
						*pOutput |= nBitValue << nBitsLeft;
					} else {
						nBitsLeft += 32 - nBitLength;
						*pOutput |= nBitValue >> (32 - nBitsLeft);
						pOutput++;
						nEncoded++;
						*pOutput |= nBitValue << nBitsLeft;
					}
				}
			}
			return nEncoded + ((nBitsLeft != 32)?1:0);
		}
};
#endif//C3WIREENCODER_H
