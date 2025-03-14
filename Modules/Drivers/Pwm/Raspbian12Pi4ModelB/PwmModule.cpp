//AbstractionLayer
#include "PwmModule.hpp"
//C++
#include <cstdio>
#include <string>
//Raspbian
#include <pigpio.h>

ErrorType Pwm::init() {
    ErrorType error = ErrorType::Failure;

    if (gpioInitialise() >= 0) {
        error = ErrorType::Success;
    }

    return error;
}

ErrorType Pwm::deinit() {
    if (gpioTerminate() >= 0) {
        return ErrorType::Success;
    }
    return ErrorType::Failure;
}

ErrorType Pwm::start() {
    uint8_t dutyCycle = (_dutyCycle / 100) * _period;
    if (0 == gpioPWM(_pinNumber, dutyCycle)) {
        return ErrorType::Success;
    }

    return ErrorType::Failure;
}

ErrorType Pwm::stop() {
    return ErrorType::NotImplemented;
}

ErrorType Pwm::setDutyCycle(Percent on) {
    _dutyCycle = on;
    return ErrorType::Success;
}

ErrorType Pwm::setPeriod(Milliseconds frequency) {
    _period = frequency;
    return ErrorType::Success;
}

ErrorType Pwm::executeOperatingSystemShellCommand(const char *command) {
    ErrorType error = ErrorType::Failure;
    char commandReturnData[32] = {0};

    FILE* pipe = popen(command, "r");
    if (nullptr != pipe) {
        if (nullptr != fgets(commandReturnData, sizeof(commandReturnData), pipe)) {
            error = ErrorType::Success;
        }
        else {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    return error;
}