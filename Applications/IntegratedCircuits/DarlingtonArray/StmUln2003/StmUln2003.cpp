#include "StmUln2003.hpp"

ErrorType StmUln2003::init() {
    ErrorType error = ErrorType::Failure;
    Count pwmIndex = 0;

    if (this->isDrivenByGpio()) {
        error = ErrorType::NotSupported;
    }
    else if (this->isDrivenByGptmPwm()) {

        for (auto &gptPwm : this->_gptPwms) {

            if (pwmIndex < params().numberOfPinsUsed) {
                gptPwm.peripheralNumber() = params().pwmPeripherals[pwmIndex];
                gptPwm.outputPin() = params().pinNumbers[pwmIndex];
                gptPwm.setPeriod(params().pwmPeriods[pwmIndex]);
                gptPwm.setDutyCycle(params().pwmDutyCycles[pwmIndex]);
                error = gptPwm.init();

                if (ErrorType::Success == error) {
                    error = gptPwm.stop();
                }

                pwmIndex++;
            }
        }
    }
    else if (this->isDrivenByStandalonePwm()) {

        for (auto &pwm : this->_pwms) {

            if (pwmIndex < params().numberOfPinsUsed) {
                pwm.peripheralNumber() = params().pwmPeripherals[pwmIndex];
                pwm.outputPin() = params().pinNumbers[pwmIndex];
                pwm.setPeriod(params().pwmPeriods[pwmIndex]);
                pwm.setDutyCycle(params().pwmDutyCycles[pwmIndex]);
                error = pwm.init();

                if (ErrorType::Success == error) {
                    error = pwm.stop();
                }

                pwmIndex++;
            }
        }
    }
    else {
        error = ErrorType::InvalidParameter;
    }

    return error;
}

ErrorType StmUln2003::togglePin(Count pinNumber, bool on) {
    if (pinNumber >= StmUln2003Types::InputPins) {
        return ErrorType::InvalidParameter;
    }

    if (isDrivenByGptmPwm()) {
        return on ? _gptPwms[pinNumber].start() : _gptPwms[pinNumber].stop();
    }
    else if (isDrivenByStandalonePwm()) {
        return on ? _pwms[pinNumber].start() : _pwms[pinNumber].stop();
    }
    else if (isDrivenByGpio()) {
        return _gpios[pinNumber].pinWrite(on ? GpioTypes::LogicLevel::High : GpioTypes::LogicLevel::Low);
    }
    else {
        return ErrorType::NotSupported;
    }
}