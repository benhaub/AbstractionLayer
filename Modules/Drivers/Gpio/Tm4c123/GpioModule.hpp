#ifndef __GPIO_MODULE_HPP__
#define __GPIO_MODULE_HPP__

//AbstractionLayer
#include "GpioAbstraction.hpp"

class Gpio final : public GpioAbstraction {

    public:
    Gpio() : GpioAbstraction() {}

    ErrorType init() override;
    ErrorType pinWrite(const GpioTypes::LogicLevel &logicLevel) override;
    ErrorType pinRead(GpioTypes::LogicLevel &logicLevel) override;
};

#endif // __GPIO_MODULE_HPP__