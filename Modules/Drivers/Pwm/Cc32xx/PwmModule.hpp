#ifndef __PWM_MODULE_HPP__
#define __PWM_MODULE_HPP__

//AbstractionLayer
#include "PwmAbstraction.hpp"
//TI posix
#include <stdint.h>
//TI sysconfig (auto-generated)
#include "ti_drivers_config.h"
//TI driverlib
#include "ti/drivers/PWM.h"

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

    private:
    PWM_Handle _pwmHandle = nullptr;

    uint_least8_t toCc32xxPwmPeripheralNumber(PwmConfig::PeripheralNumber peripheral, ErrorType &error) {
        uint_least8_t cc32xxPwmPeripheralNumber = 0;
        error = ErrorType::Success;

        switch(peripheral) {
            case PwmConfig::PeripheralNumber::Zero:
                cc32xxPwmPeripheralNumber = 0;
                break;
            case PwmConfig::PeripheralNumber::One:
                cc32xxPwmPeripheralNumber = 1;
                break;
            case PwmConfig::PeripheralNumber::Two:
                cc32xxPwmPeripheralNumber = 2;
                break;
            case PwmConfig::PeripheralNumber::Three:
                cc32xxPwmPeripheralNumber = 3;
                break;
            case PwmConfig::PeripheralNumber::Four:
                cc32xxPwmPeripheralNumber = 4;
                break;
            case PwmConfig::PeripheralNumber::Five:
                cc32xxPwmPeripheralNumber = 5;
                break;
            case PwmConfig::PeripheralNumber::Six:
                cc32xxPwmPeripheralNumber = 6;
                break;
            default:
                error = ErrorType::InvalidParameter;
        }

        if (cc32xxPwmPeripheralNumber > CONFIG_TI_DRIVERS_PWM_COUNT) {
            error = ErrorType::InvalidParameter;
        }

        return cc32xxPwmPeripheralNumber;
    }
};

#endif // __PWM_MODULE_HPP__