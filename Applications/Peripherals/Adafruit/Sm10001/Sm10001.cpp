#include "Sm10001.hpp"
//AbstractionLayer
#include "OperatingSystemModule.hpp"
#include "Math.hpp"

ErrorType Sm10001::init() {
    if (Sm10001Drivers::InputSignalType::Gpio == _inputSignalType) {
        return ErrorType::NotSupported;
    }
    else if (Sm10001Drivers::InputSignalType::PwmStandalone == _inputSignalType) {
        std::vector<Pwm> pwms;
        pwms.reserve(_MotorInputPins);
        pwms.emplace_back(Pwm());
        //TODO: The motor should not be deciding what peripherals it uses. That is application specific. If I really
        //want to streamline the init process for PWMs I should make a facade for it by creating an initDefault function or create a
        //factory for PWMs with GPTM and standalone driver as the paramter.
        pwms.at(0).peripheralNumber() = PeripheralNumber::Zero;
        pwms.emplace_back(Pwm());
        pwms.at(1).peripheralNumber() = PeripheralNumber::One;

        for (auto &pwm: pwms) {
            pwm.setPeriod(_PwmPeriod);
            pwm.setDutyCycle(_PwmDutyCycle);
            ErrorType error;
            pwm.outputPin() = mapPeripheralToPinNumber(pwm.peripheralNumber(), error);
            assert(ErrorType::Success == error);
            error = pwm.init();
            if (ErrorType::Success != error) {
                const bool isCriticalError = !(ErrorType::NotAvailable == error);
                if (isCriticalError) {
                    return error;
                }
            }
            else {
                pwm.stop();
            }
        }

        _hBridge->setPwms(pwms);
    }
    else if (Sm10001Drivers::InputSignalType::PwmTimer == _inputSignalType) {
        std::vector<GptmPwmModule> gptPwms;
        gptPwms.reserve(_MotorInputPins);
        gptPwms.emplace_back(GptmPwmModule());
        gptPwms.at(0).peripheralNumber() = PeripheralNumber::Zero;
        gptPwms.emplace_back(GptmPwmModule());
        gptPwms.at(1).peripheralNumber() = PeripheralNumber::One;

        for (auto &gptPwm : gptPwms) {
            gptPwm.setPeriod(_PwmPeriod);
            gptPwm.setDutyCycle(_PwmDutyCycle);
            ErrorType error;
            gptPwm.outputPin() = mapPeripheralToPinNumber(gptPwm.peripheralNumber(), error);
            assert(ErrorType::Success == error);
            error = gptPwm.init();
            if (ErrorType::Success != error) {
                const bool isCriticalError = !(ErrorType::NotAvailable == error);
                if (isCriticalError) {
                    return error;
                }
            }
            else {
                gptPwm.stop();
            }
        }

        _hBridge->setPwms(gptPwms);
    }

    return ErrorType::Success;
}

ErrorType Sm10001::slideForward() {
    ErrorType error = _hBridge->driveForward();
    OperatingSystem::Instance().delay(Milliseconds(_PwmPeriod / 2));
    if (ErrorType::Success == error) {
        _hBridge->coast();
    }
    return error;
}

ErrorType Sm10001::slideBackward() {
    ErrorType error = _hBridge->driveBackward();
    OperatingSystem::Instance().delay(Milliseconds(_PwmPeriod / 2));
    if (ErrorType::Success == error) {
        _hBridge->coast();
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