#include "IConfigRunner.h"

//Stub implementations
IConfigRunner::~IConfigRunner() {
}
void IConfigRunner::beginModule(const char *szName, const char *szDescription) {
}
void IConfigRunner::endModule() {
}
void IConfigRunner::optionBool(const char *szName, const char *szDescription, bool *pValue, bool bDefault) {
}
void IConfigRunner::optionString(const char *szName, const char *szDescription, char *szValue, size_t nSize, const char *szDefault) {
}
void IConfigRunner::optionInt(const char *szName, const char *szDescription, void *pValue, size_t nSize, uint32_t nMin, uint32_t nMax, uint32_t nDefault) {
}
