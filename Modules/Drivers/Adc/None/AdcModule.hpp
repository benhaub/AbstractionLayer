#ifndef __ADC_MODULE_HPP__
#define __ADC_MODULE_HPP__

//AbstractionLayer
#include "AdcAbstraction.hpp"

class Adc : public AdcAbstraction {
    public:
    Adc() = default;

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType convert(Count &rawValue) override;
    ErrorType rawToVolts(const Count rawValue, Volts &volts) override;
};

#endif //__ADC_MODULE_HPP__