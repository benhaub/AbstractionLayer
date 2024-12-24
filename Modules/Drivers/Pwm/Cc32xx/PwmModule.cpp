//AbstractionLayer
#include "PwmModule.hpp"

ErrorType Pwm::init() {
    return ErrorType::NotImplemented;
}

ErrorType Pwm::setHardwareConfig(PeripheralNumber peripheral) {
    return ErrorType::NotImplemented;
}

ErrorType Pwm::setDriverConfig(Percent duty, Milliseconds period) {
    return ErrorType::NotImplemented;
}

ErrorType Pwm::setFirmwareConfig() {
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

ErrorType Pwm::setPeriod(Milliseconds period) {
    return ErrorType::NotImplemented;
}