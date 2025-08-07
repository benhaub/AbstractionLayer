#include "GpioModule.hpp"
//Common
#include "Log.hpp"

ErrorType Gpio::init() {
    ErrorType error = toEspGpioConfig(gpioParams());

    if (ErrorType::Success == error) {
        error = fromPlatformError(gpio_config(&_gpioConfig));
    }

    return error;
}

ErrorType Gpio::pinWrite(const GpioTypes::LogicLevel &logicLevel) {
    if (GpioTypes::LogicLevel::High == logicLevel) {
        return fromPlatformError(gpio_set_level(toEspPinNumber(gpioParams().hardwareConfig.pinNumber), 1));
    }
    else if (GpioTypes::LogicLevel::Low == logicLevel) {
        return fromPlatformError(gpio_set_level(toEspPinNumber(gpioParams().hardwareConfig.pinNumber), 0));
    }
    else {
        return ErrorType::InvalidParameter;
    }

    return ErrorType::Success;
}

ErrorType Gpio::pinRead(GpioTypes::LogicLevel &logicLevel) {
    if (1 == gpio_get_level(toEspPinNumber(gpioParams().hardwareConfig.pinNumber))) {
        logicLevel = GpioTypes::LogicLevel::High;
    }
    else {
        logicLevel = GpioTypes::LogicLevel::Low;
    }

    return ErrorType::Success;
}

ErrorType Gpio::toEspGpioConfig(const GpioTypes::GpioParams &params) {

    if (gpioParams().interruptConfig.interruptFlags & GpioTypes::Interrupts::FallingEdge) {
        _gpioConfig.intr_type = GPIO_INTR_NEGEDGE;
    }
    else if (gpioParams().interruptConfig.interruptFlags & GpioTypes::Interrupts::RisingEdge) {
       _gpioConfig.intr_type = GPIO_INTR_POSEDGE;
    }
    else if (gpioParams().interruptConfig.interruptFlags & GpioTypes::Interrupts::RisingOrFallingEdge) {
        _gpioConfig.intr_type = GPIO_INTR_ANYEDGE;
    }
    else if (gpioParams().interruptConfig.interruptFlags & GpioTypes::Interrupts::HighLevel) {
        _gpioConfig.intr_type = GPIO_INTR_HIGH_LEVEL;
    }
    else if (gpioParams().interruptConfig.interruptFlags & GpioTypes::Interrupts::LowLevel) {
        _gpioConfig.intr_type = GPIO_INTR_LOW_LEVEL;
    }
    else if (gpioParams().interruptConfig.interruptFlags == GpioTypes::Interrupts::RisingOrFallingEdge) {
        _gpioConfig.intr_type = GPIO_INTR_ANYEDGE;
    }
    else if (gpioParams().interruptConfig.interruptFlags == GpioTypes::Interrupts::Disabled) {
        _gpioConfig.intr_type = GPIO_INTR_DISABLE;
    }
    else {
        return ErrorType::InvalidParameter;
    }

    switch (gpioParams().hardwareConfig.direction) {
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
    _gpioConfig.pin_bit_mask = ((uint32_t)1 << gpioParams().hardwareConfig.pinNumber);

    if (gpioParams().hardwareConfig.pullDownEnable && gpioParams().hardwareConfig.pullUpEnable) {
        return ErrorType::InvalidParameter;
    }

    if (gpioParams().hardwareConfig.pullDownEnable) {
        _gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
    }
    else {
        _gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    }

    if (gpioParams().hardwareConfig.pullUpEnable) {
        _gpioConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    }
    else {
        _gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    }

    return ErrorType::Success;
}