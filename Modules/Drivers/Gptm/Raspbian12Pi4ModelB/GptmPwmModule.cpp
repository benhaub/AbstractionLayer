#include "GptmPwmModule.hpp"
//Raspbian
#include <pigpio.h>

ErrorType GptmPwmModule::init() {
    ErrorType error = ErrorType::Failure;

    if (gpioInitialise() >= 0) {
        error = ErrorType::Success;
    }

    return error;
}

ErrorType GptmPwmModule::deinit() {
    if (gpioTerminate() >= 0) {
        return ErrorType::Success;
    }
    return ErrorType::Failure;
}

ErrorType GptmPwmModule::start() { 
    const uint32_t dutyValue = (_dutyCycle / 100) * gpioGetPWMrange(outputPinConst());
    if (0 == gpioPWM(outputPinConst(), dutyValue)) {
        return ErrorType::Success;
    }

    return ErrorType::Failure;
}

ErrorType GptmPwmModule::stop() {
    if (0 == gpioPWM(outputPinConst(), 0)) {
        return ErrorType::Success;
    }

    return ErrorType::Failure;
}

ErrorType GptmPwmModule::setDutyCycle(const Percent on) {
    _dutyCycle = on;
    return ErrorType::Success;
}

ErrorType GptmPwmModule::setPeriod(const Microseconds period) {
    _period = period;

    const int closestAvailableFrequency = gpioSetPWMfrequency(outputPinConst(), 1/(period * 1E-6));
    if (PI_BAD_USER_GPIO != closestAvailableFrequency) {
        return ErrorType::Success;
    }

    return ErrorType::Failure;
}