//AbstractionLayer
#include "ProcessorModule.hpp"
//CMSIS
#include "cmsis_compiler.h"

ErrorType Processor::isInterruptContext() {
    return (__get_IPSR() != 0) ? ErrorType::Success : ErrorType::Negative;
}

ErrorType Processor::interruptRequestNumber(uint32_t &interruptRequestNumber) {
    interruptRequestNumber = __get_IPSR();
    return ErrorType::Success;
}

ErrorType Processor::stackPointer(uint32_t &stackPointer) {
    stackPointer = __get_PSP();
    return ErrorType::Success;
}