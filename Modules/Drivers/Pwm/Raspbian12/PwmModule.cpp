//AbstractionLayer
#include "PwmModule.hpp"
//C++
#include <cstdio>
#include <string>

ErrorType Pwm::init() {
    ErrorType error = ErrorType::Failure;
    std::string initCommand = "python3 " PATH_TO_SCRIPTS " /init.py ";
    initCommand.append(std::to_string(toRaspbian12PeripheralNumber(_peripheral, error)));

    if (ErrorType::Success == error) {
        initCommand.push_back(' ');
        initCommand.append(std::to_string(_period));
        initCommand.push_back(' ');
        initCommand.append(std::to_string(_dutyCycle));
    }

    return executeOperatingSystemShellCommand(initCommand.c_str());
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