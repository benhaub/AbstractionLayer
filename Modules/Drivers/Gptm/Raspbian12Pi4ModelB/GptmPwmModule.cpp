#include "GptmPwmModule.hpp"
//Raspbian
#include <pigpio.h>

GptmPwmModule::GptmPwmModule() : GptmPwmAbstraction() {
    gpioInitialise();
}

ErrorType GptmPwmModule::init() {
    return ErrorType::Success;
}

ErrorType GptmPwmModule::deinit() {
    gpioTerminate();
    return ErrorType::Success;
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
