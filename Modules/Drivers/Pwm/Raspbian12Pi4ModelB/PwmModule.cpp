//AbstractionLayer
#include "PwmModule.hpp"
//C++
#include <cstdio>
#include <string>

ErrorType Pwm::init() {
    return ErrorType::NotImplemented;
}

ErrorType Pwm::deinit() {
    return ErrorType::NotImplemented;
}

ErrorType Pwm::start() {
    return ErrorType::NotImplemented;
}

ErrorType Pwm::stop() {
    return ErrorType::NotImplemented;
}

ErrorType Pwm::setDutyCycle(Percent on) {
    return ErrorType::NotImplemented;
}

ErrorType Pwm::setPeriod(Milliseconds frequency) {
    return ErrorType::NotImplemented;
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