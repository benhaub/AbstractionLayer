#ifndef __PWM_MODULE_HPP__
#define __PWM_MODULE_HPP__

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

    private:
    ErrorType executeOperatingSystemShellCommand(const char *command);

    uint8_t toRaspbian12PeripheralNumber(PeripheralNumber peripheral, ErrorType &error) const {
        error = ErrorType::Success;

        switch (peripheral) {
            case PeripheralNumber::Zero:
                return 0;
            case PeripheralNumber::One:
                return 1;
            default:
                error = ErrorType::InvalidParameter;
                return 0;
        }
    }
};

#endif // __PWM_MODULE_HPP__