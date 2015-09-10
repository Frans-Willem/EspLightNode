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
