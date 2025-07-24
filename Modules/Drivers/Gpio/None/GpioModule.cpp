#include "GpioModule.hpp"

ErrorType Gpio::init() {
    return ErrorType::NotImplemented;
}

ErrorType Gpio::setHardwareConfig(const Register basePeripheralRegister, const PinNumber pinNumber, const GpioTypes::PinDirection direction, const GpioTypes::InterruptMode interruptMode, const bool pullUpEnable, const bool pullDownEnable) {
    return ErrorType::NotImplemented;
}

ErrorType Gpio::pinWrite(const GpioTypes::LogicLevel &logicLevel) {
    return ErrorType::NotImplemented;
}

ErrorType Gpio::pinRead(GpioTypes::LogicLevel &logicLevel) {
    return ErrorType::NotImplemented;
}