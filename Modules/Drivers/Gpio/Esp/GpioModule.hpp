#ifndef __GPIO_MODULE_HPP__
#define __GPIO_MODULE_HPP__

//AbstractionLayer
#include "GpioAbstraction.hpp"
//Standard library
#include <stdint.h>
//ESP
#include "hal/gpio_types.h"
#include "driver/gpio.h"

class Gpio final : public GpioAbstraction {

    public:
    Gpio() : GpioAbstraction() {}

    ErrorType init() override;
    ErrorType setHardwareConfig(const Register basePeripheralRegister, const PinNumber pinNumber, const GpioTypes::PinDirection direction, const GpioTypes::InterruptMode interruptMode, const bool pullUpEnable, const bool pullDownEnable) override;
    ErrorType pinWrite(const GpioTypes::LogicLevel &logicLevel) override;
    ErrorType pinRead(GpioTypes::LogicLevel &logicLevel) override;

    gpio_num_t toEspPinNumber(const PinNumber pinNumber) {
        return static_cast<gpio_num_t>(pinNumber);
    }

    private:
    gpio_config_t _gpioConfig;
};

#endif // __GPIO_MODULE_HPP__
