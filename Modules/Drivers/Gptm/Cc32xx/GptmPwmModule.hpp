#ifndef __GPTM_PWM_MODULE_HPP__
#define __GPTM_PWM_MODULE_HPP__

#include "GptmPwmAbstraction.hpp"
//TI sysconfig (auto-generated)
#include "ti_drivers_config.h"
//TI driverlib
#include "ti/drivers/PWM.h"

class GptmPwmModule final : public GptmPwmAbstraction {

    public:
    GptmPwmModule() : GptmPwmAbstraction() {}

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType start() override;
    ErrorType stop() override;
    ErrorType setDutyCycle(const Percent on) override;
    ErrorType setPeriod(const Microseconds period) override;

    private:
    PWM_Handle _pwmHandle = nullptr;

    uint_least8_t toCc32xxPwmPeripheralNumber(PeripheralNumber peripheral, ErrorType &error) {
        uint_least8_t cc32xxPwmPeripheralNumber = 0;
        error = ErrorType::Success;

        switch(peripheral) {
            case PeripheralNumber::Zero:
#ifdef CONFIG_PWM_0
                cc32xxPwmPeripheralNumber = CONFIG_PWM_0;
#else
                error = ErrorType::InvalidParameter;
#endif
                break;
            case PeripheralNumber::One:
#ifdef CONFIG_PWM_1
                cc32xxPwmPeripheralNumber = CONFIG_PWM_1;
#else
                error = ErrorType::InvalidParameter;
#endif
                break;
            case PeripheralNumber::Two:
#ifdef CONFIG_PWM_2
                cc32xxPwmPeripheralNumber = CONFIG_PWM_2;
#else
                error = ErrorType::InvalidParameter;
#endif
                break;
            case PeripheralNumber::Three:
#ifdef CONFIG_PWM_3
                cc32xxPwmPeripheralNumber = CONFIG_PWM_3;
#else
                error = ErrorType::InvalidParameter;
#endif
                break;
            case PeripheralNumber::Four:
#ifdef CONFIG_PWM_4
                cc32xxPwmPeripheralNumber = CONFIG_PWM_4;
#else
                error = ErrorType::InvalidParameter;
#endif
                break;
            case PeripheralNumber::Five:
#ifdef CONFIG_PWM_5
                cc32xxPwmPeripheralNumber = CONFIG_PWM_5;
#else
                error = ErrorType::InvalidParameter;
#endif
                break;
            case PeripheralNumber::Six:
#ifdef CONFIG_PWM_6
                cc32xxPwmPeripheralNumber = CONFIG_PWM_6;
#else
                error = ErrorType::InvalidParameter;
#endif
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