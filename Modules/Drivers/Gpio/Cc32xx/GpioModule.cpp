//AbstractionLayer
#include "GpioModule.hpp"
//TI driverlib
#include "ti/drivers/GPIO.h"

ErrorType Gpio::pinWrite(const GpioLogicLevel &logicLevel) {
    return ErrorType::NotImplemented;
}

ErrorType Gpio::pinRead(GpioLogicLevel &logicLevel) {
    return ErrorType::NotImplemented;
}

ErrorType Gpio::configure(const uint32_t *basePeripheralRegister, const PinNumber pinNumber, const GpioPinDirection direction, const GpioInterruptMode interruptMode, const bool pullUpEnable, const bool pullDownEnable) {
    //Idempotent init.
    GPIO_init();
    return ErrorType::Success;
}