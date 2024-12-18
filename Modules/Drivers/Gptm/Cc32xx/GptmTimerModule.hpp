#ifndef __GPTM_TIMER_MODULE_HPP__
#define __GPTM_TIMER_MODULE_HPP__

#include "GptmTimerAbstraction.hpp"

class GptmTimerModule : public GptmTimerAbstraction {
    GptmTimerModule() : GptmTimerAbstraction() {}
    ~GptmTimerModule() = default;

    ErrorType init() override;
    ErrorType setHardwareConfig(PeripheralIdentifier identifier, void * peripheral) override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;
};

#endif //__GPTM_TIMER_MODULE_HPP__