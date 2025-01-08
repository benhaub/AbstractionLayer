#ifndef __GPTM_PWM_MODULE_HPP__
#define __GPTM_PWM_MODULE_HPP__

#include "GptmPwmAbstraction.hpp"

class GptmPwmModule : public GptmPwmAbstraction {

    public:
    GptmPwmModule() : GptmPwmAbstraction() {}
    ~GptmPwmModule() = default;

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;

    ErrorType setDutyCycle(const Percent on) override;
    ErrorType setPeriod(const Milliseconds period) override;
};

#endif //__GPTM_PWM_MODULE_HPP__