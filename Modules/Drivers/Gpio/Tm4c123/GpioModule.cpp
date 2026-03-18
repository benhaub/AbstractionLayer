#include "GpioModule.hpp"
//TI sysconfig
#include "pinout.h"

ErrorType Gpio::init() {
    PinoutSet();
    return ErrorType::Success;
}

ErrorType Gpio::pinWrite(const GpioTypes::LogicLevel &logicLevel) {
    return ErrorType::NotImplemented;
}

ErrorType Gpio::pinRead(GpioTypes::LogicLevel &logicLevel) {
    return ErrorType::NotImplemented;
}