//AbstractionLayer
#include "Sm10001.hpp"
#include "OperatingSystemModule.hpp"
#include "Math.hpp"

ErrorType Sm10001::slideForward(const Milliseconds slideTime) {

    return std::visit([&](auto &hBridge) -> ErrorType {
        const Count numIterations = (slideTime*1000) / hBridge.params().pwmPeriod;
        ErrorType error = ErrorType::Success;;

        for (Count i = 0; i < numIterations; i++) {
            error = hBridge.driveForward();

            if (ErrorType::Success == error) {
                OperatingSystem::Instance().delay(hBridge.params().pwmPeriod / 2);
                error = hBridge.brake();
            }
            else {
                break;
            }
        }

        return error;

    }, *_hBridge);
}

ErrorType Sm10001::slideBackward(const Milliseconds slideTime) {

    return std::visit([&](auto &hBridge) -> ErrorType {
        const Count numIterations = (slideTime*1000) / hBridge.params().pwmPeriod;
        ErrorType error = ErrorType::Success;

        for (Count i = 0; i < numIterations; i++) {
            ErrorType error = hBridge.driveBackward();

            if (ErrorType::Success == error) {
                OperatingSystem::Instance().delay(hBridge.params().pwmPeriod / 2);
                error = hBridge.brake();
            }
            else {
                break;
            }
        }

        return error;

    }, *_hBridge);
}

ErrorType Sm10001::slideToVoltage(const Percent ofMaxVoltage, const Volts hysteresis) {
    Volts currentReading = 0.0f;
    ErrorType error = getVoltageDrop(currentReading, Sm10001Types::AdcMultiSamples);
    const Volts desired = (ofMaxVoltage / 100.0f) * _maxVoltage;

    if (ErrorType::Success == error) {
        if (0 != _minimumForwardSlideTime && 0 != _minimumBackwardSlideTime) {

            if (Sm10001Types::ForwardSlideVoltageEffect::Unknown != _forwardSlideVoltageEffect) {
                while (!withinError(desired, currentReading, hysteresis) && ErrorType::Success == error) {
                    if (currentReading < desired) {

                        if (_forwardSlideVoltageEffect == Sm10001Types::ForwardSlideVoltageEffect::Raises) {
                            error = slideForward(_minimumForwardSlideTime);
                        }
                        else {
                            error = slideBackward(_minimumBackwardSlideTime);
                        }
                    }
                    else {
                        if (_forwardSlideVoltageEffect == Sm10001Types::ForwardSlideVoltageEffect::Drops) {
                            error = slideForward(_minimumForwardSlideTime);
                        }
                        else {
                            error = slideBackward(_minimumBackwardSlideTime);
                        }
                    }

                    error = getVoltageDrop(currentReading, Sm10001Types::AdcMultiSamples);
                }
            }
            else {
                error = ErrorType::PrerequisitesNotMet;
            }
        }
        else {
            error = ErrorType::PrerequisitesNotMet;
        }
    }

    return error;
}

ErrorType Sm10001::setSpeed(const Percent speed) {
    if (speed < 0.0f || speed > 100.0f) {
        return ErrorType::InvalidParameter;
    }

    return std::visit([&](auto &hBridge) -> ErrorType {
        ErrorType error = hBridge.changeDutyCycle(speed);

        if (ErrorType::Success == error) {
            _speed = speed;
        }

        return error;
    }, *_hBridge);
}

ErrorType Sm10001::getVoltageDrop(Volts &voltageDrop, const Count multiSamples) {
    Count rawAdcValue = 0;
    Volts currentReading = 0.0f;
    ErrorType error = ErrorType::Failure;

    for (Count i = 0; i < multiSamples; i++) {
        error = _adc.convert(rawAdcValue);

        if (ErrorType::Success == error) {
            error = _adc.rawToVolts(rawAdcValue, currentReading);

            if (ErrorType::Success == error) {
                voltageDrop = runningAverage(voltageDrop, currentReading, i);
            }
        }
    }

    return error;
}

ErrorType Sm10001::calibrate(const Count numRetries, const Volts hysteresis, const Milliseconds slideTime) {
    ErrorType error = calibrateMinimumForwardSlideTime(_minimumForwardSlideTime, hysteresis);

    if (ErrorType::Success == error) {
        error = calibrateMinimumBackwardSlideTime(_minimumBackwardSlideTime, hysteresis);
    }

    return error;
}

ErrorType Sm10001::calibrateMinimumForwardSlideTime(Milliseconds &minimumForwardSlideTime, const Volts hysteresis) {
    minimumForwardSlideTime = 0;
    Volts potentiometerVoltageDropPrevious = 0.0f;
    Volts potentiometerVoltageDropNow = 0.0f;
    Volts differenceBetweenNowAndPrevious = 0.0f;

    ErrorType error = getVoltageDrop(potentiometerVoltageDropNow, Sm10001Types::AdcMultiSamples);

    if (ErrorType::Success == error) {
        potentiometerVoltageDropPrevious = potentiometerVoltageDropNow;

        while (differenceBetweenNowAndPrevious < hysteresis) {
            minimumForwardSlideTime++;
            error = slideForward(minimumForwardSlideTime);

            if (ErrorType::Success == error) {
                error = getVoltageDrop(potentiometerVoltageDropNow, Sm10001Types::AdcMultiSamples);

                //In case the wiper is positioned all the way to one end such that it can't slide forward.
                if (withinError(potentiometerVoltageDropNow, _maxVoltage, hysteresis) || withinError(potentiometerVoltageDropNow, _minVoltage, hysteresis)) {
                    slideBackward(minimumForwardSlideTime*32);

                    if (!(withinError(potentiometerVoltageDropNow, _maxVoltage, hysteresis) || withinError(potentiometerVoltageDropNow, _minVoltage, hysteresis))) {
                        minimumForwardSlideTime = 0;
                    }
                }
                else {
                    differenceBetweenNowAndPrevious = std::abs(potentiometerVoltageDropNow - potentiometerVoltageDropPrevious);
                }

                potentiometerVoltageDropNow > potentiometerVoltageDropPrevious ? _forwardSlideVoltageEffect = Sm10001Types::ForwardSlideVoltageEffect::Raises : Sm10001Types::ForwardSlideVoltageEffect::Drops;
            }

            if (ErrorType::Success != error) {
                break;
            }
        }
    }

    return error;
}

ErrorType Sm10001::calibrateMinimumBackwardSlideTime(Milliseconds &minimumBackwardSlideTime, const Volts hysteresis) {
    minimumBackwardSlideTime = 0;
    Volts potentiometerVoltageDropPrevious = 0.0f;
    Volts potentiometerVoltageDropNow = 0.0f;
    Volts differenceBetweenNowAndPrevious = 0.0f;

    ErrorType error = getVoltageDrop(potentiometerVoltageDropNow, Sm10001Types::AdcMultiSamples);
    
    if (ErrorType::Success == error) {
        potentiometerVoltageDropPrevious = potentiometerVoltageDropNow;

        while (differenceBetweenNowAndPrevious < hysteresis) {
            minimumBackwardSlideTime++;
            error = slideBackward(minimumBackwardSlideTime);

            if (ErrorType::Success == error) {
                error = getVoltageDrop(potentiometerVoltageDropNow, Sm10001Types::AdcMultiSamples);

                //In case the wiper is positioned all the way to one end such that it can't slide backward.
                if (withinError(potentiometerVoltageDropNow, _maxVoltage, hysteresis) || withinError(potentiometerVoltageDropNow, _minVoltage, hysteresis)) {
                    error = slideForward(minimumBackwardSlideTime*32);

                    if (ErrorType::Success == error) {

                        if (!(withinError(potentiometerVoltageDropNow, _maxVoltage, hysteresis) || withinError(potentiometerVoltageDropNow, _minVoltage, hysteresis))) {
                            minimumBackwardSlideTime = 0;
                        }
                    }
                }
                else {
                    differenceBetweenNowAndPrevious = std::abs(potentiometerVoltageDropNow - potentiometerVoltageDropPrevious);
                }

            }

            if (ErrorType::Success != error) {
                break;
            }
        }
    }

    return error;
}