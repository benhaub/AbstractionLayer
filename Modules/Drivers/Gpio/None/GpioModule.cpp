#include "GpioModule.hpp"

ErrorType Gpio::init() {
    return ErrorType::NotImplemented;
}

ErrorType Gpio::pinWrite(const GpioTypes::LogicLevel &logicLevel) const {
    return ErrorType::NotImplemented;
}

ErrorType Gpio::pinRead(GpioTypes::LogicLevel &logicLevel) const {
    return ErrorType::NotImplemented;
}