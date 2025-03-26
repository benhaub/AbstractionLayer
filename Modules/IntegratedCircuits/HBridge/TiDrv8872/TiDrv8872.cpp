#include "TiDrv8872.hpp"

ErrorType TiDrv8872::driveForward() {
    ErrorType error = ErrorType::Failure;
    assert(isDrivenByGptmPwm() || isDrivenByStandalonePwm());

    if (isDrivenByGptmPwm()) {
        error = _gptPwms[0]->start();
        if (ErrorType::Success == error) {
            error = _gptPwms[1]->stop();
        }
    }
    else if (isDrivenByStandalonePwm()) {
        error = _pwms[0]->start();
        if (ErrorType::Success == error) {
            error = _pwms[1]->stop();
        }
    }
    else if (isDrivenByGpio()) {
        error = _gpios[0]->pinWrite(GpioTypes::LogicLevel::High);
        if (ErrorType::Success == error) {
            error = _gpios[1]->pinWrite(GpioTypes::LogicLevel::Low);
        }
    }
    else {
        error = ErrorType::NotSupported;
    }

    return error;
}

ErrorType TiDrv8872::driveBackward() {
    ErrorType error = ErrorType::Failure;
    assert(isDrivenByGptmPwm() || isDrivenByStandalonePwm());

    if (isDrivenByGptmPwm()) {
        error = _gptPwms[0]->stop();
        if (ErrorType::Success == error) {
            error = _gptPwms[1]->start();
        }
    }
    else if (isDrivenByStandalonePwm()) {
        error = _pwms[0]->stop();
        if (ErrorType::Success == error) {
            error = _pwms[1]->start();
        }
    }
    else if (isDrivenByGpio()) {
        error = _gpios[0]->pinWrite(GpioTypes::LogicLevel::Low);
        if (ErrorType::Success == error) {
            error = _gpios[1]->pinWrite(GpioTypes::LogicLevel::High);
        }
    }
    else {
        error = ErrorType::NotSupported;
    }

    return error;
}

ErrorType TiDrv8872::coast() {
    ErrorType error = ErrorType::Failure;
    assert(isDrivenByGptmPwm() || isDrivenByStandalonePwm());

    if (isDrivenByGptmPwm()) {
        error = _gptPwms[0]->stop();
        if (ErrorType::Success == error) {
            error = _gptPwms[1]->stop();
        }
    }
    else if (isDrivenByStandalonePwm()) {
        error = _pwms[0]->stop();
        if (ErrorType::Success == error) {
            error = _pwms[1]->stop();
        }
    }
    else if (isDrivenByGpio()) {
        error = _gpios[0]->pinWrite(GpioTypes::LogicLevel::Low);
        if (ErrorType::Success == error) {
            error = _gpios[1]->pinWrite(GpioTypes::LogicLevel::Low);
        }
    }
    else {
        error = ErrorType::NotSupported;
    }

    return error;
}

ErrorType TiDrv8872::brake() {
    ErrorType error = ErrorType::Failure;
    assert(isDrivenByGptmPwm() || isDrivenByStandalonePwm());

    if (isDrivenByGptmPwm()) {
        error = _gptPwms[0]->start();
        if (ErrorType::Success == error) {
            error = _gptPwms[1]->start();
        }
    }
    else if (isDrivenByStandalonePwm()) {
        error = _pwms[0]->start();
        if (ErrorType::Success == error) {
            error = _pwms[1]->start();
        }
    }
    else if (isDrivenByGpio()) {
        error = _gpios[0]->pinWrite(GpioTypes::LogicLevel::High);
        if (ErrorType::Success == error) {
            error = _gpios[1]->pinWrite(GpioTypes::LogicLevel::High);
        }
    }
    else {
        error = ErrorType::NotSupported;
    }

    return error;
}