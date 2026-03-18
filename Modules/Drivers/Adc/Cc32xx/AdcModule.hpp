#ifndef __ADC_MODULE_HPP__
#define __ADC_MODULE_HPP__

//AbstractionLayer
#include "AdcAbstraction.hpp"
//TI sysconfig (auto-generated)
#include "ti_drivers_config.h"
//TI Drivers
#include "ti/drivers/ADC.h"

class Adc final : public AdcAbstraction {
    public:
    Adc() = default;

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType convert(Count &rawValue) override;
    ErrorType rawToVolts(const Count rawValue, Volts &volts) override;

    private:
    ADC_Handle _handle = nullptr;

    uint_least8_t toCc32xxAdcPeripheralNumber(PeripheralNumber peripheral, ErrorType &error) {
        uint_least8_t cc32xxAdcPeripheralNumber = 0;
        error = ErrorType::Success;

        switch(peripheral) {
            case PeripheralNumber::Zero:
#ifdef CONFIG_ADC_0
                cc32xxAdcPeripheralNumber = CONFIG_ADC_0;
#else
                error = ErrorType::InvalidParameter;
#endif
                break;
            case PeripheralNumber::One:
#ifdef CONFIG_ADC_1
                cc32xxAdcPeripheralNumber = CONFIG_ADC_1;
#else
                error = ErrorType::InvalidParameter;
#endif
                break;
            case PeripheralNumber::Two:
#ifdef CONFIG_ADC_2
                cc32xxAdcPeripheralNumber = CONFIG_ADC_2;
#else
                error = ErrorType::InvalidParameter;
#endif
                break;
            case PeripheralNumber::Three:
#ifdef CONFIG_ADC_3
                cc32xxAdcPeripheralNumber = CONFIG_ADC_3;
#else
                error = ErrorType::InvalidParameter;
#endif
                break;
            case PeripheralNumber::Four:
#ifdef CONFIG_ADC_4
                cc32xxAdcPeripheralNumber = CONFIG_ADC_4;
#else
                error = ErrorType::InvalidParameter;
#endif
                break;
            case PeripheralNumber::Five:
#ifdef CONFIG_ADC_5
                cc32xxAdcPeripheralNumber = CONFIG_ADC_5;
#else
                error = ErrorType::InvalidParameter;
#endif
                break;
            case PeripheralNumber::Six:
#ifdef CONFIG_ADC_6
                cc32xxAdcPeripheralNumber = CONFIG_ADC_6;
#else
                error = ErrorType::InvalidParameter;
#endif
                break;
            default:
                error = ErrorType::InvalidParameter;
        }
#ifdef CONFIG_TI_DRIVERS_ADC_COUNT
        if (cc32xxAdcPeripheralNumber >= CONFIG_TI_DRIVERS_ADC_COUNT) {
            error = ErrorType::InvalidParameter;
        }
#else
        error = ErrorType::NotAvailable;
#endif

        return cc32xxAdcPeripheralNumber;
    }
};

#endif //__ADC_MODULE_HPP__