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

// We should actually find out why this is linked in, but for now, stubs
/*
extern "C" void _sbrk_r() { while(1); }
extern "C" void _write_r() { while(1); }
extern "C" void _read_r() { while(1); }
extern "C" void _fstat_r() { while(1); }
extern "C" void _exit() { while(1); }
extern "C" void _kill_r() { while(1); }
extern "C" void _open_r() { while(1); }
extern "C" void _close_r() { while(1); }
extern "C" void _getpid_r() { while(1); }
extern "C" void _lseek_r() { while(1); }
*/
extern "C" void abort() {
	while (1);
}
