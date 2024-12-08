#ifndef __PWM_MODULE_HPP__
#define __PWM_MODULE_HPP__

#include "PwmAbstraction.hpp"

class Pwm : public PwmAbstraction {
    public:
    Pwm() : PwmAbstraction() {}
    ~Pwm() = default;

    ErrorType init() override;
    ErrorType setHardwareConfig(PwmConfig::PeripheralNumber peripheral) override;
    ErrorType setDriverConfig(Percent duty, Hertz frequency) override;
    ErrorType setFirmwareConfig() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;
    ErrorType setDutyCycle(Percent on) override;
    ErrorType setFrequency(Hertz frequency) override;
};

#endif // __PWM_MODULE_HPP__