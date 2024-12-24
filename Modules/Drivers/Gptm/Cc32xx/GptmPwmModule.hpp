#ifndef __GPTM_PWM_MODULE_HPP__
#define __GPTM_PWM_MODULE_HPP__

#include "GptmPwmAbstraction.hpp"
//TI sysconfig (auto-generated)
#include "ti_drivers_config.h"
//TI driverlib
#include "ti/drivers/PWM.h"

class GptmPwmModule : public GptmPwmAbstraction {

    public:
    GptmPwmModule() : GptmPwmAbstraction() {}
    ~GptmPwmModule() = default;

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;
    ErrorType setDutyCycle(Percent on) override;
    ErrorType setPeriod(Milliseconds period) override;

    private:
    PWM_Handle _pwmHandle = nullptr;

    uint_least8_t toCc32xxPwmPeripheralNumber(PeripheralNumber peripheral, ErrorType &error) {
        uint_least8_t cc32xxPwmPeripheralNumber = 0;
        error = ErrorType::Success;

        switch(peripheral) {
            case PeripheralNumber::Zero:
                cc32xxPwmPeripheralNumber = 0;
                break;
            case PeripheralNumber::One:
                cc32xxPwmPeripheralNumber = 1;
                break;
            case PeripheralNumber::Two:
                cc32xxPwmPeripheralNumber = 2;
                break;
            case PeripheralNumber::Three:
                cc32xxPwmPeripheralNumber = 3;
                break;
            case PeripheralNumber::Four:
                cc32xxPwmPeripheralNumber = 4;
                break;
            case PeripheralNumber::Five:
                cc32xxPwmPeripheralNumber = 5;
                break;
            case PeripheralNumber::Six:
                cc32xxPwmPeripheralNumber = 6;
                break;
            default:
                error = ErrorType::InvalidParameter;
        }

        if (cc32xxPwmPeripheralNumber >= CONFIG_TI_DRIVERS_PWM_COUNT) {
            error = ErrorType::InvalidParameter;
        }

        return cc32xxPwmPeripheralNumber;
    }
};

#endif //__GPTM_PWM_MODULE_HPP__