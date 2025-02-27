#include "Sm10001.hpp"

ErrorType Sm10001::init() {
    PeripheralNumber peripheral = PeripheralNumber::Unknown;
    constexpr uint8_t numberOfInputs = 2;
    constexpr Milliseconds pwmPeriod = Milliseconds(200);
    constexpr Percent pwmDutyCycle = Percent(25);

    for (int i = 0; i < numberOfInputs; i++) {
        _pwmIsImplementedByGptm = _gptPwms[i].init() != ErrorType::NotImplemented;
        _pwmIsStandaloneDriver = _pwms[i].init() != ErrorType::NotImplemented;

        switch (i) {
            case 0: peripheral = PeripheralNumber::Zero;
                break;
            case 1: peripheral = PeripheralNumber::One;
                break;
        }

        if (_pwmIsStandaloneDriver) {
            _pwms[i].peripheralNumber() = peripheral;
            _pwms[i].setPeriod(pwmPeriod);
            _pwms[i].setDutyCycle(pwmDutyCycle);
            ErrorType error = _pwms[i].init();
            if (ErrorType::Success != error) {
                const bool isCriticalError = !(ErrorType::NotAvailable == error || ErrorType::NotImplemented == error);
                if (isCriticalError) {
                    return error;
                }
            }
            else {
                _pwms[i].stop();
            }
        }
        else if (_pwmIsImplementedByGptm) {
            _gptPwms[i].peripheralNumber() = peripheral;
            _gptPwms[i].setPeriod(pwmPeriod);
            _gptPwms[i].setDutyCycle(pwmDutyCycle);
            ErrorType error = _gptPwms[i].init();
            if (ErrorType::Success != error) {
                const bool isCriticalError = !(ErrorType::NotAvailable == error || ErrorType::NotImplemented == error);
                if (isCriticalError) {
                    return error;
                }
            }
            else {
                _gptPwms[i].stop();
            }
        }
    }
}