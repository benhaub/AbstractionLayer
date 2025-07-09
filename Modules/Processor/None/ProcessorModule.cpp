//AbstractionLayer
#include "ProcessorModule.hpp"

ErrorType Processor::isInterruptContext() {
    return ErrorType::NotImplemented;
}
ErrorType Processor::interruptRequestNumber(uint32_t &interruptRequestNumber) {
    return ErrorType::NotImplemented;
}
ErrorType Processor::stackPointer(uint32_t &stackPointer) {
    return ErrorType::NotImplemented;
}