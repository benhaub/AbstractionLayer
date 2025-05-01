#ifndef __PROCESSOR_MODULE_HPP__
#define __PROCESSOR_MODULE_HPP__

//AbstractionLayer
#include "ProcessorAbstraction.hpp"
#include "Global.hpp"

class Processor final : public ProcessorAbstraction, public Global<Processor> {
    public:
    Processor() = default;

    ErrorType isInterruptContext() override;
    ErrorType interruptRequestNumber(uint32_t &interruptRequestNumber) override;
    ErrorType stackPointer(uint32_t &stackPointer) override;
};

#endif //__PROCESSOR_MODULE_HPP__
