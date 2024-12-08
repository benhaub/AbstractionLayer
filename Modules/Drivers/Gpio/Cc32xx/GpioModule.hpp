#ifndef __GPIO_MODULE_HPP__
#define __GPIO_MODULE_HPP__

//AbstractionLayer
#include "GpioAbstraction.hpp"
//C++
#include <cstdint>

class Gpio : public GpioAbstraction {

    public:
    Gpio() : GpioAbstraction() {}
    ~Gpio() = default;

    ErrorType pinWrite(const GpioLogicLevel &logicLevel) override;
    ErrorType pinRead(GpioLogicLevel &logicLevel) override;
    ErrorType configure(const uint32_t *basePeripheralRegister, const PinNumber pinNumber, const GpioPinDirection direction, const GpioInterruptMode interruptMode, const bool pullUpEnable, const bool pullDownEnable) override;
};

#endif // __GPIO_MODULE_HPP__