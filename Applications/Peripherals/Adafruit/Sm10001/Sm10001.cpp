//AbstractionLayer
#include "Sm10001.hpp"
#include "OperatingSystemModule.hpp"

ErrorType Sm10001::slideForward() {

    return std::visit([&](auto &hBridge) -> ErrorType {
        ErrorType error = hBridge.driveForward();

        if (ErrorType::Success == error) {
            OperatingSystem::Instance().delay(Milliseconds(75));
            error = hBridge.brake();
        }

        return error;

    }, *_hBridge);
}

ErrorType Sm10001::slideBackward() {

    return std::visit([&](auto &hBridge) -> ErrorType {
        ErrorType error = hBridge.driveBackward();

        if (ErrorType::Success == error) {
            OperatingSystem::Instance().delay(Milliseconds(75));
            error = hBridge.brake();
        }

        return error;

    }, *_hBridge);
}

ErrorType Sm10001::getVoltageDrop(Volts &voltageDrop) {
    Count rawAdcValue = 0;

    ErrorType error = _adc.convert(rawAdcValue);

    if (ErrorType::Success == error) {
        error = _adc.rawToVolts(rawAdcValue, voltageDrop);
    }

    return error;
}