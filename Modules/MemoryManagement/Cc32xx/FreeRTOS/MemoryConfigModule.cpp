//C++
#include <cassert>
//FreeRTOS
#include "FreeRTOS.h"
static void *allocate(size_t size) {
    void *ptr = pvPortMalloc(size);

    assert(nullptr != ptr);
    return ptr;
}

void *operator new(size_t size) {
    return allocate(size);
}

void *operator new[](size_t size) {
    return allocate(size);
}

void operator delete(void *ptr) {
    vPortFree(ptr);
}
void operator delete(void *ptr, unsigned int) {
    vPortFree(ptr);
}

void operator delete[](void *ptr) {
    vPortFree(ptr);
}
void operator delete [](void *ptr, unsigned int) {
    vPortFree(ptr);
}