#ifndef __GPIO_MODULE_HPP__
#define __GPIO_MODULE_HPP__

//AbstractionLayer
#include "GpioAbstraction.hpp"
//C++
#include <cstdint>

class Gpio final : public GpioAbstraction {

    public:
    Gpio() : GpioAbstraction() {}

    ErrorType init() override;
    ErrorType setHardwareConfig(const uint32_t *basePeripheralRegister, const PinNumber pinNumber, const GpioTypes::PinDirection direction, const GpioTypes::InterruptMode interruptMode, const bool pullUpEnable, const bool pullDownEnable) override;
    ErrorType pinWrite(const GpioTypes::LogicLevel &logicLevel) override;
    ErrorType pinRead(GpioTypes::LogicLevel &logicLevel) override;
};

#endif // __GPIO_MODULE_HPP__