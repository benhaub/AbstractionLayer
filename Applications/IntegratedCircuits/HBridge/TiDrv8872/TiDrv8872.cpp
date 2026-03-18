#include "TiDrv8872.hpp"
//C++
#include <cassert>

ErrorType TiDrv8872::init() {
    ErrorType error = ErrorType::InvalidParameter;

    if (isDrivenByGptmPwm()) {
        _gptPwms[0].peripheralNumber() = params().pwm1;
        _gptPwms[1].peripheralNumber() = params().pwm2;
        _gptPwms[0].outputPin() = params().input1;
        _gptPwms[1].outputPin() = params().input2;

        for (auto &gptPwm : _gptPwms) {
            error = gptPwm.setPeriod(params().pwmPeriod);

            if (ErrorType::Success == error) {
                error = gptPwm.setDutyCycle(params().pwmDutyCycle);

                if (ErrorType::Success == error) {
                    error = gptPwm.init();
                }
            }
        }
    }
    else if (isDrivenByStandalonePwm()) {
        _pwms[0].peripheralNumber() = params().pwm1;
        _pwms[1].peripheralNumber() = params().pwm2;
        _pwms[0].outputPin() = params().input1;
        _pwms[1].outputPin() = params().input2;

        for (auto &pwm : _pwms) {
            error = pwm.setPeriod(params().pwmPeriod);

            if (ErrorType::Success == error) {
                error = pwm.setDutyCycle(params().pwmDutyCycle);

                if (ErrorType::Success == error) {
                    error = pwm.init();
                }
            }
        }
    }
    else if (isDrivenByGpio()) {
        error = ErrorType::NotSupported;
    }

    return error;
}

ErrorType TiDrv8872::driveForward() {
    ErrorType error = ErrorType::Failure;
    assert(isDrivenByGptmPwm() || isDrivenByStandalonePwm());

    if (isDrivenByGptmPwm()) {
        error = _gptPwms[0].start();
        if (ErrorType::Success == error) {
            error = _gptPwms[1].stop();
        }
    }
    else if (isDrivenByStandalonePwm()) {
        error = _pwms[0].start();
        if (ErrorType::Success == error) {
            error = _pwms[1].stop();
        }
    }
    else if (isDrivenByGpio()) {
        error = _gpios[0].pinWrite(GpioTypes::LogicLevel::High);
        if (ErrorType::Success == error) {
            error = _gpios[1].pinWrite(GpioTypes::LogicLevel::Low);
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
        error = _gptPwms[0].stop();
        if (ErrorType::Success == error) {
            error = _gptPwms[1].start();
        }
    }
    else if (isDrivenByStandalonePwm()) {
        error = _pwms[0].stop();
        if (ErrorType::Success == error) {
            error = _pwms[1].start();
        }
    }
    else if (isDrivenByGpio()) {
        error = _gpios[0].pinWrite(GpioTypes::LogicLevel::Low);
        if (ErrorType::Success == error) {
            error = _gpios[1].pinWrite(GpioTypes::LogicLevel::High);
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
        error = _gptPwms[0].stop();
        if (ErrorType::Success == error) {
            error = _gptPwms[1].stop();
        }
    }
    else if (isDrivenByStandalonePwm()) {
        error = _pwms[0].stop();
        if (ErrorType::Success == error) {
            error = _pwms[1].stop();
        }
    }
    else if (isDrivenByGpio()) {
        error = _gpios[0].pinWrite(GpioTypes::LogicLevel::Low);
        if (ErrorType::Success == error) {
            error = _gpios[1].pinWrite(GpioTypes::LogicLevel::Low);
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
        error = _gptPwms[0].start();
        if (ErrorType::Success == error) {
            error = _gptPwms[1].start();
        }
    }
    else if (isDrivenByStandalonePwm()) {
        error = _pwms[0].start();
        if (ErrorType::Success == error) {
            error = _pwms[1].start();
        }
    }
    else if (isDrivenByGpio()) {
        error = _gpios[0].pinWrite(GpioTypes::LogicLevel::High);
        if (ErrorType::Success == error) {
            error = _gpios[1].pinWrite(GpioTypes::LogicLevel::High);
        }
    }
    else {
        error = ErrorType::NotSupported;
    }

    return error;
}