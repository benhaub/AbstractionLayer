#ifndef __GPTM_TIMER_MODULE_HPP__
#define __GPTM_TIMER_MODULE_HPP__

#include "GptmTimerAbstraction.hpp"

class GptmTimerModule final : public GptmTimerAbstraction {
    GptmTimerModule() : GptmTimerAbstraction() {}

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;
};

#endif //__GPTM_TIMER_MODULE_HPP__