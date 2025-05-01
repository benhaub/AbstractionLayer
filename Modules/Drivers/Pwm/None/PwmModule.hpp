#ifndef __PWM_MODULE_HPP__
#define __PWM_MODULE_HPP__

#include "PwmAbstraction.hpp"

class Pwm : public PwmAbstraction {
    public:
    Pwm() : PwmAbstraction() {}

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;
    ErrorType setDutyCycle(Percent on) override;
    ErrorType setPeriod(const Microseconds period) override;
};

#endif // __PWM_MODULE_HPP__