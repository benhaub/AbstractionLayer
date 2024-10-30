#ifndef __GPIO_MODULE_HPP__
#define __GPIO_MODULE_HPP__

//Foundation
#include "GpioAbstraction.hpp"
//Standard library
#include <stdint.h>
//ESP
#include "hal/gpio_types.h"

class Gpio : public GpioAbstraction {

    public:
    Gpio() : GpioAbstraction() {}
    virtual ~Gpio() = default;

    ErrorType pinWrite(const GpioLogicLevel &logicLevel) override;
    ErrorType pinRead(GpioLogicLevel &logicLevel) override;
    ErrorType configure(const uint32_t *basePeripheralRegister, const PinNumber pinNumber, const GpioPinDirection direction, const GpioInterruptMode interruptMode, const bool pullUpEnable, const bool pullDownEnable) override;

    gpio_num_t toEspPinNumber(const PinNumber pinNumber) {
        return static_cast<gpio_num_t>(pinNumber);
    }
};

#endif // __GPIO_MODULE_HPP__