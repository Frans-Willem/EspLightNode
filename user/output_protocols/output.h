#ifndef OUTPUT_PROTOCOLS_OUTPUT_H
#define OUTPUT_PROTOCOLS_OUTPUT_H
#include <stdint.h>
#include <stddef.h>

void output_init();
void output(const uint8_t *pData, size_t nLength);
#endif//OUTPUT_PROTOCOLS_OUTPUT_H
