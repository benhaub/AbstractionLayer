//AbstractionLayer
#include "GpioModule.hpp"
//TI driverlib
#include "ti/drivers/GPIO.h"

ErrorType Gpio::init() {
    //Idempotent init.
    GPIO_init();
    return ErrorType::Success;
}

ErrorType Gpio::setHardwareConfig(const uint32_t *basePeripheralRegister, const PinNumber pinNumber, const GpioTypes::PinDirection direction, const GpioTypes::InterruptMode interruptMode, const bool pullUpEnable, const bool pullDownEnable) {
    return ErrorType::NotImplemented;
}

ErrorType Gpio::pinWrite(const GpioTypes::LogicLevel &logicLevel) {
    return ErrorType::NotImplemented;
}

ErrorType Gpio::pinRead(GpioTypes::LogicLevel &logicLevel) {
    return ErrorType::NotImplemented;
}