#ifndef __GPTM_PWM_MODULE_HPP__
#define __GPTM_PWM_MODULE_HPP__

#include "GptmPwmAbstraction.hpp"

class GptmPwmModule final : public GptmPwmAbstraction {

    public:
    GptmPwmModule() : GptmPwmAbstraction() {}

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;

    ErrorType setDutyCycle(const Percent on) override;
    ErrorType setPeriod(const Microseconds period) override;
};

#endif //__GPTM_PWM_MODULE_HPP__