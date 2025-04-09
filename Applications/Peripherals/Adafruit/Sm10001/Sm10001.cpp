#include "Sm10001.hpp"
//AbstractionLayer
#include "OperatingSystemModule.hpp"
#include "Math.hpp"

ErrorType Sm10001::init() {
    ErrorType error = ErrorType::Failure;

    if (_hBridge->isDrivenByGpio()) {
        error = ErrorType::NotSupported;
    }
    else if (_hBridge->isDrivenByStandalonePwm()) {
        _hBridge->standalonePwms().at(0)->outputPin() = _motorInputA;
        _hBridge->standalonePwms().at(1)->outputPin() = _motorInputB;

        for (auto &pwm: _hBridge->standalonePwms()) {
            if (ErrorType::Success == (error = pwm->setPeriod(_PwmPeriod))) {
                if (ErrorType::Success == (error = pwm->setDutyCycle(_PwmDutyCycle))) {
                    error = pwm->init();
                }
            }

            error = pwm->stop();
        }
    }
    else if (_hBridge->isDrivenByGptmPwm()) {
        _hBridge->gptPwms().at(0)->outputPin() = _motorInputA;
        _hBridge->gptPwms().at(1)->outputPin() = _motorInputB;

        for (auto &gptPwm : _hBridge->gptPwms()) {
            if (ErrorType::Success == (error = gptPwm->setPeriod(_PwmPeriod))) {
                if (ErrorType::Success == (error = gptPwm->setDutyCycle(_PwmDutyCycle))) {
                    error = gptPwm->init();
                }
            }

            error = gptPwm->stop();
        }
    }

    return error;
}

ErrorType Sm10001::slideForward() {
    ErrorType error = _hBridge->driveForward();

    OperatingSystem::Instance().delay(Milliseconds(75));

    if (ErrorType::Success == error) {
        error = _hBridge->brake();
    }

    return error;
}

ErrorType Sm10001::slideBackward() {
    ErrorType error = _hBridge->driveBackward();

    OperatingSystem::Instance().delay(Milliseconds(75));

    if (ErrorType::Success == error) {
        error = _hBridge->brake();
    }
    return error;
}

ErrorType Sm10001::getVoltageDrop(Volts &voltageDrop) {
    Count rawAdcValue = 0;

    ErrorType error = _adc->convert(rawAdcValue);
    if (ErrorType::Success == error) {
        error = _adc->rawToVolts(rawAdcValue, voltageDrop);
    }

    return error;
}