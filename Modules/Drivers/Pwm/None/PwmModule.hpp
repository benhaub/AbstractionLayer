#ifndef __PWM_MODULE_HPP__
#define __PWM_MODULE_HPP__

#include "PwmAbstraction.hpp"

class Pwm : public PwmAbstraction {
    public:
    Pwm() : PwmAbstraction() {}
    ~Pwm() = default;

    ErrorType init() override;
    ErrorType setHardwareConfig(PwmConfig::PeripheralNumber peripheral) override;
    ErrorType setDriverConfig(Percent duty, Milliseconds period) override;
    ErrorType setFirmwareConfig() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;
    ErrorType setDutyCycle(Percent on) override;
    ErrorType setPeriod(Milliseconds period) override;
};

#endif // __PWM_MODULE_HPP__