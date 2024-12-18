#ifndef __GPTM_PWM_MODULE_HPP__
#define __GPTM_PWM_MODULE_HPP__

#include "GptmPwmAbstraction.hpp"

class GptmPwmModule : public GptmPwmAbstraction {
    GptmPwmModule() : GptmPwmAbstraction() {}
    ~GptmPwmModule() = default;

    ErrorType start() override;
    ErrorType stop() override;
};

#endif //__GPTM_PWM_MODULE_HPP__