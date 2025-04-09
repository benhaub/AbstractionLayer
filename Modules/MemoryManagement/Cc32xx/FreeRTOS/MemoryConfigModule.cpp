//C++
#include <cassert>
//FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

static void *allocate(size_t size) {
    void *ptr = nullptr;

    if (0 == size) {
        return nullptr;
    }

    ptr = pvPortMalloc(size);

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

#ifdef __cplusplus
extern "C" {
#endif

void vApplicationMallocFailedHook(void) {
    assert(false);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    assert(false);
}

#ifdef __cplusplus
}
#endif

//*****************************************************************************
//
//! \brief  Overwrite the GCC _sbrk function which check the heap limit related
//!         to the stack pointer.
//!         In case of freertos this checking will fail.
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
#if defined (__GNUC__)
void * _sbrk(uint32_t delta)
{
    extern char _end;     /* Defined by the linker */
    extern char __HeapLimit;
    static char *heap_end;
    static char *heap_limit;
    char *prev_heap_end;

    if(heap_end == 0)
    {
        heap_end = &_end;
        heap_limit = &__HeapLimit;
    }

    prev_heap_end = heap_end;
    if(prev_heap_end + delta > heap_limit)
    {
        return((void *) -1L);
    }
    heap_end += delta;
    return((void *) prev_heap_end);
}

#endif