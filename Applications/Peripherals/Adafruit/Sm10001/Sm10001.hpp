/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   Sm10001.hpp
* @details Driver for the slide potentiometer by adafruit
* @see https://www.adafruit.com/product/5466
* @ingroup Applications
*******************************************************************************/
#ifndef __SM10001_HPP__
#define __SM10001_HPP__

//AbstractionLayer
#include "AdcModule.hpp"
#include "HBridgeFactory.hpp"
#include "OperatingSystemModule.hpp"
#include "Math.hpp"

/**
 * @namespace Sm10001Types
 * @brief Contains types related to the slide potentiometer by adafruit.
 */
namespace Sm10001Types {
    /// @brief Period of the PWMs for the motor.
    constexpr Microseconds PwmPeriod = 1000;
    /// @brief Duty cycle of the PWMs for the motor.
    constexpr Percent PwmDutyCycle = 75;
    /// @brief The number of times to multisample the ADC.
    constexpr Count AdcMultiSamples = 30;

    static_assert(1000 / PwmPeriod <= 250, "Frequency is beyond max recommended in data sheet.");

    /// @brief Tag for logging
    constexpr char TAG[] = "Sm10001";

    /**
     * @enum ForwardSlideVoltageEffect
     * @brief The effect that a forward slide has in terms of voltage drop
     */
    enum class ForwardSlideVoltageEffect : uint8_t {
        Unknown = 0, ///< Unknown effect
        Drops,       ///< Sliding forward decreases the voltage drop.
        Raises       ///< Sliding forward increases the voltage drop.
    };
}

/**
 * @class Sm10001
 * @brief Driver for the slide potentiometer by adafruit
 */
template <typename T, HBridgeFactoryTypes::PartNumber _PartNumber, HBridgeTypes::PwmType _PwmType, PeripheralNumber _AdcPeripheralNumber, AdcTypes::Channel _Channel,
          PinNumber _MotorInputA, PinNumber _MotorInputB, T _MaxVoltageDrop, T _MinVoltageDrop>
          requires(std::is_same_v<T, Volts>)
class Sm10001 {

    public:
    /**
     * @brief init the PWMs for the motor
     * @returns ErrorType::Success if the PWMs were initialized
     * @returns ErrorType::Failure otherwise
     */
    ErrorType init() {

        ErrorType error = HBridgeFactory::Factory<_PartNumber>(_hBridge);

        if (ErrorType::Success == error) {
            std::visit([&](auto &hBridge) {
                HBridgeTypes::ConfigurationParameters params;
                params.pwmType = _PwmType;
                params.input1 = _MotorInputA;
                params.input2 = _MotorInputB;
                params.pwm1 = PeripheralNumber::Zero;
                params.pwm2 = PeripheralNumber::One;
                params.pwmPeriod = Sm10001Types::PwmPeriod;
                params.pwmDutyCycle = _speed = Sm10001Types::PwmDutyCycle;
                hBridge.configure(params);

                error = hBridge.init();

            }, *_hBridge);

            if (ErrorType::Success == error) {
                AdcTypes::Parameters adcParams;
                adcParams.channel = _Channel;
                adcParams.peripheralNumber = _AdcPeripheralNumber;
                _adc.configure(adcParams);

                error = _adc.init();
            }
        }

        return error;
    }
    /**
     * @brief Slide forward
     * @param slideTime The amount of time to slide forward for.
     * @returns ErrorType::Success if the slide was successful
     * @returns ErrorType::Failure otherwise
     */
    ErrorType slideForward(const Milliseconds slideTime) {
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
    /**
     * @brief Slide backward
     * @param slideTime The amount of time to slide backward for.
     * @returns ErrorType::Success if the slide was successful
     * @returns ErrorType::Failure otherwise
     */
    ErrorType slideBackward(const Milliseconds slideTime) {
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
    /**
     * @brief Slide the wiper to the desired voltage drop reading.
     * @param ofMaxVoltage The desired voltage drop as a percentage of the maximum possible
     * @param hysteresis The amount of difference in the voltage that is fed back that is acceptable.
     */
    ErrorType slideToVoltage(const Percent ofMaxVoltage, const Volts hysteresis) {
        Volts currentReading = 0.0f;
        ErrorType error = getVoltageDrop(currentReading, Sm10001Types::AdcMultiSamples);
        const Volts desired = (ofMaxVoltage / 100.0f) * maxVoltage();

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
    /**
     * @brief Set the speed at which the slide moves
     * @param speed The speed at which the slide moves
     * @returns ErrorType::Success if the speed was set successfully
     * @returns ErrorType::Failure otherwise
     */
    ErrorType setSpeed(const Percent speed) {
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
    /**
     * @brief Get the voltage drop reading from the potentiometer
     * @param[out] volts The voltage drop reading will be stored here
     * @param[in] multiSamples To mitigate noise, use the number of multisamples and return the average.
     * @returns ErrorType::Success if the reading was successful
     * @returns ErrorType::Failure otherwise
     * @post volts is invalid only if ErrorType::Failure is returned.
     */
    ErrorType getVoltageDrop(Volts &voltageDrop, const Count multiSamples) {
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
    /**
     * @brief Measures and saves the minimum and maximum voltage values that represent 100% and 0% voltage drop of the potentiometer.
     *        Also measures the average voltage that is changed when sliding forwards and backwards.
     * @param[in] numRetries The amount of times to retry if the voltage doesn't change.
     * @param[in] hysteresis The smallest acceptable amount of voltage to consider a change.
     * @param[in] slideTime The amount of time to drive the H-Bridge in the forward and backward direction.
     * @details If calibration fails, try increasing slideTime first, then numRetries, then hysteresis.
     * @returns ErrorType::Success if the calibration was successful
     * @returns Any errors returned by slideBackward, slideForward, getVoltageDrop
     */
    ErrorType calibrate(const Count numRetries, const Volts hysteresis, const Milliseconds slideTime) {
        ErrorType error = calibrateMinimumForwardSlideTime(_minimumForwardSlideTime, hysteresis);

        if (ErrorType::Success == error) {
            error = calibrateMinimumBackwardSlideTime(_minimumBackwardSlideTime, hysteresis);
        }

        return error;
    }

    /// @brief Get the minimum voltage as a constant reference
    constexpr T minVoltage() const { return _MinVoltageDrop; }
    /// @brief Get the maximum voltage as a constant reference
    constexpr T maxVoltage() const { return _MaxVoltageDrop; }
    /// @brief Get the speed as a constant reference
    const Percent &speed() const { return _speed; }
    /// @brief Get the minimum forward slide time as a constant reference
    const Milliseconds &minimumForwardSlideTime() const { return _minimumForwardSlideTime; }
    /// @brief Get the minimum backward slide time as a constant reference
    const Milliseconds &minimumBackwardSlideTime() const { return _minimumBackwardSlideTime; }
    /// @brief Get the effect that a forward slide has on the voltage drop as a constant reference
    const Sm10001Types::ForwardSlideVoltageEffect &forwardSlideVoltageEffect() const { return _forwardSlideVoltageEffect; }

    private:
    /// @brief The H-Bridge that drives the motor
    std::optional<HBridgeFactoryTypes::HBridgeFactoryVariant> _hBridge;
    /// @brief The ADC that reads the potentiometer voltage drop.
    Adc _adc;
    /// @brief The percentage of the maximum speed at which the slide moves.
    Percent _speed = 0.0f;
    /// @brief The minimum slide time that needs to be applied to slideForward to cause a voltage change.
    Milliseconds _minimumForwardSlideTime = 0;
    /// @brief The minimum slide time that needs to be applied to slideBackward to cause a voltage change.
    Milliseconds _minimumBackwardSlideTime = 0;
    /// @brief The effect that a forward slide has on the voltage of the potentiometer
    Sm10001Types::ForwardSlideVoltageEffect _forwardSlideVoltageEffect = Sm10001Types::ForwardSlideVoltageEffect::Unknown; 

    /**
     * @brief Determine the minimum number of milliseconds needed to move the wiper forward at the current speed.
     * @param[out] minimumForwardSlideTime The minimum amount of time needed to slide forward.
     * @param[in] hysteresis The minimum acceptable voltage to count as a change.
     * @returns ErrorType::Success if the calibration was successfull
     * @returns ErrorType::Failure otherwise.
     */
    ErrorType calibrateMinimumForwardSlideTime(Milliseconds &minimumForwardSlideTime, const Volts hysteresis) {
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
                    if (withinError(potentiometerVoltageDropNow, maxVoltage(), hysteresis) || withinError(potentiometerVoltageDropNow, minVoltage(), hysteresis)) {
                        slideBackward(minimumForwardSlideTime*32);

                        if (!(withinError(potentiometerVoltageDropNow, maxVoltage(), hysteresis) || withinError(potentiometerVoltageDropNow, minVoltage(), hysteresis))) {
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
    /**
     * @brief Determine the minimum number of milliseconds needed to move the wiper backward at the current speed.
     * @param[out] minimumBackwardSlideTime The minimum amount of time needed to slide forward.
     * @param[in] hysteresis The minimum acceptable voltage to count as a change.
     * @returns ErrorType::Success if the calibration was successfull
     * @returns ErrorType::Failure otherwise.
     */
    ErrorType calibrateMinimumBackwardSlideTime(Milliseconds &minimumBackwardSlideTime, const Volts hysteresis) {
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
                    if (withinError(potentiometerVoltageDropNow, maxVoltage(), hysteresis) || withinError(potentiometerVoltageDropNow, minVoltage(), hysteresis)) {
                        error = slideForward(minimumBackwardSlideTime*32);

                        if (ErrorType::Success == error) {

                            if (!(withinError(potentiometerVoltageDropNow, maxVoltage(), hysteresis) || withinError(potentiometerVoltageDropNow, minVoltage(), hysteresis))) {
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
};

#endif //__SM10001_HPP__
