//AbstractionLayer
#include "ProcessorModule.hpp"
//FreeRTOS
//I would typically advise against making the processor depend on an RTOS
//but I'm going to make a bold claim that using an ESP processor without the supplied RTOS would
//be foolish especially given that Cadence documentation for Xtensa is not public. Secondly, after
//looking through what is available for xtensa core documentation, it would appear that to read the
//interrupt status from the Processor Status register requires privledged access. To work around this,
//ESP has implemented some counters inside an interrupt entry function to count interrupt nesting as
//way to get this information without requiring privledged access.
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

ErrorType Processor::isInterruptContext() {
    return pdTRUE == portCHECK_IF_IN_ISR() ? ErrorType::Success : ErrorType::Negative;
}
ErrorType Processor::interruptRequestNumber(uint32_t &interruptRequestNumber) {
    return ErrorType::NotImplemented;
}
ErrorType Processor::stackPointer(uint32_t &stackPointer) {
    return ErrorType::NotImplemented;
}