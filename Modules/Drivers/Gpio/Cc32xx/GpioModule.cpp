//AbstractionLayer
#include "GpioModule.hpp"
//TI driverlib
#include "ti/drivers/GPIO.h"

ErrorType Gpio::init() {
    //Idempotent init.
    GPIO_init();
    return ErrorType::Success;
}

ErrorType Gpio::pinWrite(const GpioTypes::LogicLevel &logicLevel) {
    return ErrorType::NotImplemented;
}

ErrorType Gpio::pinRead(GpioTypes::LogicLevel &logicLevel) {
    return ErrorType::NotImplemented;
}