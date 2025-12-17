#include "esp_heap_caps.h"
#include <cassert>
//Going off the strategies listed here: https://docs.espressif.com/projects/esp-idf/en/v5.1.1/esp32/api-reference/kconfig.html#config-spiram-malloc-alwaysinternal
//https://docs.espressif.com/projects/esp-idf/en/v5.1.1/esp32/api-reference/system/mem_alloc.html
static void *allocate(size_t size) {
#if CONFIG_SPIRAM
    constexpr size_t spiramThreshold = 8196;
#else
    //If there is no SPIRAM then increase the threshold to as large as possible so that all allocations are made from the DRAM
    constexpr size_t spiramThreshold = UINT32_MAX;
#endif
    void *ptr = nullptr;

    if (size <= spiramThreshold) {
        if (nullptr == (ptr = heap_caps_malloc(size, MALLOC_CAP_INTERNAL))) {
            return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
        }
    }
    else {
        if (nullptr == (ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM))) {
            return heap_caps_malloc(size, MALLOC_CAP_INTERNAL);
        }
    }

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
    heap_caps_free(ptr);
}
void operator delete(void *ptr, unsigned int) {
    heap_caps_free(ptr);
}

void operator delete[](void *ptr) {
    heap_caps_free(ptr);
}
void operator delete [](void *ptr, unsigned int) {
    heap_caps_free(ptr);
}
