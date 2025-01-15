#include "GpioModule.hpp"
//Common
#include "Log.hpp"

ErrorType Gpio::init() {
    if (-1 == pinNumber() || GpioTypes::PinDirection::DigitalUnknown == direction() || GpioTypes::InterruptMode::Unknown == interruptMode()) {
        return ErrorType::PrerequisitesNotMet;
    }

    return fromPlatformError(gpio_config(&_gpioConfig));
}

ErrorType Gpio::setHardwareConfig(const uint32_t *basePeripheralRegister, const PinNumber pinNumber, const GpioTypes::PinDirection direction, const GpioTypes::InterruptMode interruptMode, const bool pullUpEnable, const bool pullDownEnable) {
    switch (interruptMode) {
        case GpioTypes::InterruptMode::Disabled:
            _gpioConfig.intr_type = GPIO_INTR_DISABLE;
            break;
        case GpioTypes::InterruptMode::FallingEdge:
            _gpioConfig.intr_type = GPIO_INTR_NEGEDGE;
            break;
        case GpioTypes::InterruptMode::HighLevel:
            _gpioConfig.intr_type = GPIO_INTR_HIGH_LEVEL;
            break;
        case GpioTypes::InterruptMode::LowLevel:
            _gpioConfig.intr_type = GPIO_INTR_LOW_LEVEL;
            break;
        case GpioTypes::InterruptMode::RisingEdge:
            _gpioConfig.intr_type = GPIO_INTR_POSEDGE;
            break;
        case GpioTypes::InterruptMode::RisingOrFallingEdge:
            _gpioConfig.intr_type = GPIO_INTR_ANYEDGE;
            break;
        default:
            return ErrorType::InvalidParameter;
    }

    switch (direction) {
        case GpioTypes::PinDirection::DigitalInput:
            _gpioConfig.mode = GPIO_MODE_INPUT;
            break;
        case GpioTypes::PinDirection::DigitalOutput:
            _gpioConfig.mode = GPIO_MODE_OUTPUT;
            break;
        default:
            return ErrorType::InvalidParameter;
    }

    //TODO: Invalid pin number?
    _gpioConfig.pin_bit_mask = ((uint32_t)1 << pinNumber);
    _pinNumber = pinNumber;

    if (pullDownEnable && pullUpEnable) {
        return ErrorType::InvalidParameter;
    }

    if (pullDownEnable) {
        _gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
    }
    else {
        _gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    }

    if (pullUpEnable) {
        _gpioConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    }
    else {
        _gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    }

    return ErrorType::Success;
}

ErrorType Gpio::pinWrite(const GpioTypes::LogicLevel &logicLevel) {
    if (GpioTypes::LogicLevel::High == logicLevel) {
        return fromPlatformError(gpio_set_level(toEspPinNumber(pinNumber()), 1));
    }
    else if (GpioTypes::LogicLevel::Low == logicLevel) {
        return fromPlatformError(gpio_set_level(toEspPinNumber(pinNumber()), 0));
    }
    else {
        return ErrorType::InvalidParameter;
    }

    return ErrorType::Success;
}

ErrorType Gpio::pinRead(GpioTypes::LogicLevel &logicLevel) {
    if (1 == gpio_get_level(toEspPinNumber(pinNumber()))) {
        logicLevel = GpioTypes::LogicLevel::High;
    }
    else {
        logicLevel = GpioTypes::LogicLevel::Low;
    }

    return ErrorType::Success;
}