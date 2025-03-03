#include "Sm10001.hpp"
//AbstractionLayer
#include "OperatingSystemModule.hpp"
#include "Math.hpp"

namespace Sm10001Types {
    /**
     * @enum InputSignalType
     * @brief The type of input signal that the SM10001 uses.
     */
    enum class InputSignal {
        Unknown = 0,   ///< Unknown input signal type
        Gpio,          ///< Input signal type is a GPIO
        PwmStandalone, ///< Input signal type is a PWM standalone
        GptPwm,        ///< Input signal type is a PWM combined
    };
}

ErrorType Sm10001::init() {
    Sm10001Types::InputSignal inputSignalType;
        ErrorType error = ErrorType::Failure;

    if (_hBridge->gpiosConst().size() > 0 && _hBridge->gpiosConst().size() == 2) {
        inputSignalType = Sm10001Types::InputSignal::Gpio;
    }
    else if (_hBridge->standalonePwms().size() > 0 && _hBridge->standalonePwms().size() == 2) {
        inputSignalType = Sm10001Types::InputSignal::PwmStandalone;
    }
    else if (_hBridge->gptPwmsConst().size() > 0 && _hBridge->gptPwmsConst().size() == 2) {
        inputSignalType = Sm10001Types::InputSignal::GptPwm;
    }
    else {
        return ErrorType::NotSupported;
    }
    
    if (Sm10001Types::InputSignal::Gpio == inputSignalType) {
        error = ErrorType::NotSupported;
    }
    else if (Sm10001Types::InputSignal::PwmStandalone == inputSignalType) {
        _hBridge->standalonePwms().at(0).outputPin() = _motorInputA;
        _hBridge->standalonePwms().at(1).outputPin() = _motorInputB;

        for (auto &pwm: _hBridge->standalonePwms()) {
            if (ErrorType::Success == (error = pwm.setPeriod(_PwmPeriod))) {
                if (ErrorType::Success == (error = pwm.setDutyCycle(_PwmDutyCycle))) {
                    error = pwm.init();
                }
            }

            error = pwm.stop();
        }
    }
    else if (Sm10001Types::InputSignal::GptPwm == inputSignalType) {
        _hBridge->gptPwms().at(0).outputPin() = _motorInputA;
        _hBridge->gptPwms().at(1).outputPin() = _motorInputB;

        for (auto &gptPwm : _hBridge->gptPwms()) {
            if (ErrorType::Success == (error = gptPwm.setPeriod(_PwmPeriod))) {
                if (ErrorType::Success == (error = gptPwm.setDutyCycle(_PwmDutyCycle))) {
                    error = gptPwm.init();
                }
            }

            error = gptPwm.stop();
        }
    }

    return error;
}

ErrorType Sm10001::slideForward() {
    ErrorType error = _hBridge->driveForward();
    OperatingSystem::Instance().delay(Milliseconds(_PwmPeriod / 2));
    if (ErrorType::Success == error) {
        error = _hBridge->coast();
    }
    return error;
}

ErrorType Sm10001::slideBackward() {
    ErrorType error = _hBridge->driveBackward();
    OperatingSystem::Instance().delay(Milliseconds(_PwmPeriod / 2));
    if (ErrorType::Success == error) {
        error = _hBridge->coast();
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