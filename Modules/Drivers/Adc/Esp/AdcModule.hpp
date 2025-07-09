#ifndef __ADC_MODULE_HPP__
#define __ADC_MODULE_HPP__

//AbstractionLayer
#include "AdcAbstraction.hpp"
//ESP
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"

class Adc : public AdcAbstraction {
    public:
    Adc() = default;

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType convert(Count &rawValue) override;
    ErrorType rawToVolts(const Count rawValue, Volts &volts) override;

    private:
    adc_oneshot_unit_handle_t adcHandle = nullptr;
    adc_cali_handle_t calibrationHandle = nullptr;

    //For pin configuration, see TRM, Pg. 68, Setc. 4.11 - RTC_MUX Pin List
    adc_channel_t toEspChannel(const AdcTypes::Channel channel, ErrorType &error) {
        error = ErrorType::Success;
        switch (channel) {
            case AdcTypes::Channel::Zero:
                return ADC_CHANNEL_0;
            case AdcTypes::Channel::One:
                return ADC_CHANNEL_1;
            case AdcTypes::Channel::Two:
                return ADC_CHANNEL_2;
            case AdcTypes::Channel::Three:
                return ADC_CHANNEL_3;
            case AdcTypes::Channel::Four:
                return ADC_CHANNEL_4;
            case AdcTypes::Channel::Five:
                return ADC_CHANNEL_5;
            case AdcTypes::Channel::Six:
                return ADC_CHANNEL_6;
            case AdcTypes::Channel::Seven:
                return ADC_CHANNEL_7;
            case AdcTypes::Channel::Eight:
                return ADC_CHANNEL_8;
            case AdcTypes::Channel::Nine:
                return ADC_CHANNEL_9;
            default:
                error = ErrorType::InvalidParameter;
                return ADC_CHANNEL_0;
        }
    }

    adc_unit_t toEspAdcUnitNumber(const PeripheralNumber peripheralNumber, ErrorType &error) {
        error = ErrorType::Success;
        switch (peripheralNumber) {
            case PeripheralNumber::Zero:
                return ADC_UNIT_1;
            case PeripheralNumber::One:
                return ADC_UNIT_2;
            default:
                error = ErrorType::InvalidParameter;
                return ADC_UNIT_1;
        }
    }
};

#endif //__ADC_MODULE_HPP__