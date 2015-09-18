#include <sdkfixup.h>
extern "C" {
#include <mem.h>
}
#include <cstddef>

void* operator new(std::size_t size) {
	return os_zalloc(size);
};

void operator delete(void *ptr) {
	os_free(ptr);
	return;
}

extern "C" void __cxa_pure_virtual() {
	while (1);
}

// These make no sense at all on ESP8266, but just in case something links to it here are stubs
extern "C" void _write_r() { while(1); }
extern "C" void _read_r() { while(1); }
extern "C" void _fstat_r() { while(1); }
extern "C" void _open_r() { while(1); }
extern "C" void _close_r() { while(1); }
extern "C" void _lseek_r() { while(1); }

// Stub
extern "C" void abort() {
	while (1);
}

// All different versions of malloc, free, realloc, etc
extern "C" void* _malloc_r(struct _reent *ignored __attribute__((unused)),
		size_t size) {
	return os_zalloc(size);
}

extern "C" void _free_r(struct _reent *ignore __attribute__((unused)), void *ptr) {
	return os_free(ptr);
}

extern "C" void* _calloc_r(struct _reent *ignored __attribute((unused)), size_t x, size_t y) {
	return os_zalloc(x*y);
}

extern "C" void* _realloc_r(struct _rrent *ignore __attribute__((unused)), void *ptr, size_t size) {
	os_free(ptr);
	return os_zalloc(size);
}

extern "C" void* malloc(size_t size) {
	return os_zalloc(size);
}

extern "C" void free(void *ptr) {
	return os_free(ptr);
}

extern "C" void* realloc(void *ptr, size_t size) {
	os_free(ptr);
	return os_zalloc(size);
}

// Relatively 'big' functions that are implemented in ROM,
// We save space by redirecting to the ROM versions.
extern "C" void *memcpy(void *dest, const void *src, size_t n) {
	return ets_memcpy(dest, src, n);
}

// strcpy, strncpy, strlen, memset are already provided by ESP8266 linker script
