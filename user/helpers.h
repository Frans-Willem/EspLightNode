#ifndef HELPERS_H
#define HELPERS_H
//Custom atof implementation, as atof on newlib pulls in a shitload of dependencies.
float eln_atof(const char *nptr);
void eln_ftoa(float fValue, char *ptr, size_t max);
int ets_snprintf(char *str, size_t size, const char *format, ...);
#endif//HELPERS_H
