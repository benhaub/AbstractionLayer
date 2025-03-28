#ifndef __PWM_MODULE_HPP__
#define __PWM_MODULE_HPP__

//AbstractionLayer
#include "PwmAbstraction.hpp"

class Pwm : public PwmAbstraction {
    public:
    Pwm() : PwmAbstraction() {}
    ~Pwm() = default;

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;
    ErrorType setDutyCycle(Percent on) override;
    ErrorType setPeriod(Milliseconds period) override;
};

#endif // __PWM_MODULE_HPP__