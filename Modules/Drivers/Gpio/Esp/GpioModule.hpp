#ifndef __GPIO_MODULE_HPP__
#define __GPIO_MODULE_HPP__

//AbstractionLayer
#include "GpioAbstraction.hpp"
//ESP
#include "hal/gpio_types.h"
#include "driver/gpio.h"

class Gpio final : public GpioAbstraction {

    public:
    Gpio() : GpioAbstraction() {}

    ErrorType init() override;
    ErrorType pinWrite(const GpioTypes::LogicLevel &logicLevel) override;
    ErrorType pinRead(GpioTypes::LogicLevel &logicLevel) override;

    gpio_num_t toEspPinNumber(const PinNumber pinNumber) {
        return static_cast<gpio_num_t>(pinNumber);
    }

    ErrorType toEspGpioConfig(const GpioTypes::GpioParams &params);

    private:
    gpio_config_t _gpioConfig;
};

#endif // __GPIO_MODULE_HPP__
