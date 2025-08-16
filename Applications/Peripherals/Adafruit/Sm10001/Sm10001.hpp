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

namespace Sm10001Types {
    /// @brief Period of the PWMs for the motor.
    constexpr Microseconds PwmPeriod = 1000;
    /// @brief Duty cycle of the PWMs for the motor.
    constexpr Percent PwmDutyCycle = 75;
    /// @brief The number of times to multisample the ADC.
    constexpr Count AdcMultiSamples = 30;

    static_assert(1000 / PwmPeriod <= 250, "Frequency is beyond max recommended in data sheet.");

    constexpr char TAG[] = "Sm10001";

    /**
     * @enum Direction
     * @brief The effect that a forward slide has in terms of voltage drop
     */
    enum class ForwardSlideVoltageEffect : uint8_t {
        Unknown = 0,
        Drops,
        Raises
    };
}

/**
 * @class Sm10001
 * @brief Driver for the slide potentiometer by adafruit
 */
class Sm10001 {

    public:
    /**
     * @brief init the PWMs for the motor
     * @returns ErrorType::Success if the PWMs were initialized
     * @returns ErrorType::Failure otherwise
     */
    ErrorType init(const HBridgeFactoryTypes::PartNumber hBridgePartNumber, const HBridgeTypes::PwmType pwmType, const PeripheralNumber adcPeripheralNumber, const AdcTypes::Channel channel,
                   const PinNumber motorInputA, const PinNumber motorInputB, const Volts maxVoltageDrop, const Volts minVoltageDrop) {

        ErrorType error = HBridgeFactory::Factory(hBridgePartNumber, _hBridge);

        if (ErrorType::Success == error) {
            std::visit([&](auto &hBridge) {
                HBridgeTypes::ConfigurationParameters params;
                params.pwmType = pwmType;
                params.input1 = _motorInputA = motorInputA;
                params.input2 = _motorInputB = motorInputB;
                params.pwm1 = PeripheralNumber::Zero;
                params.pwm2 = PeripheralNumber::One;
                params.pwmPeriod = Sm10001Types::PwmPeriod;
                params.pwmDutyCycle = _speed = Sm10001Types::PwmDutyCycle;
                hBridge.configure(params);

                error = hBridge.init();

            }, *_hBridge);

            if (ErrorType::Success == error) {
                AdcTypes::Parameters adcParams;
                adcParams.channel = channel;
                adcParams.peripheralNumber = adcPeripheralNumber;
                _adc.configure(adcParams);

                error = _adc.init();

                _maxVoltage = maxVoltageDrop;
                _minVoltage = minVoltageDrop;
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
    ErrorType slideForward(const Milliseconds slideTime);
    /**
     * @brief Slide backward
     * @param slideTime The amount of time to slide backward for.
     * @returns ErrorType::Success if the slide was successful
     * @returns ErrorType::Failure otherwise
     */
    ErrorType slideBackward(const Milliseconds slideTime);
    /**
     * @brief Slide the wiper to the desired voltage drop reading.
     * @param ofMaxVoltage The desired voltage drop as a percentage of the maximum possible
     * @param hysteresis The amount of difference in the voltage that is fed back that is acceptable.
     */
    ErrorType slideToVoltage(const Percent ofMaxVoltage, const Volts hysteresis);
    /**
     * @brief Set the speed at which the slide moves
     * @param speed The speed at which the slide moves
     * @returns ErrorType::Success if the speed was set successfully
     * @returns ErrorType::Failure otherwise
     */
    ErrorType setSpeed(const Percent speed);
    /**
     * @brief Get the voltage drop reading from the potentiometer
     * @param[out] volts The voltage drop reading will be stored here
     * @param[in] multiSamples To mitigate noise, use the number of multisamples and return the average.
     * @returns ErrorType::Success if the reading was successful
     * @returns ErrorType::Failure otherwise
     * @post volts is invalid only if ErrorType::Failure is returned.
     */
    ErrorType getVoltageDrop(Volts &volts, const Count multiSamples);
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
    ErrorType calibrate(const Count numRetries, const Volts hysteresis, const Milliseconds slideTime);

    /// @brief Get the minimum voltage as a constant reference
    const Volts &minVoltage() const { return _minVoltage; }
    /// @brief Get the maximum voltage as a constant reference
    const Volts &maxVoltage() const { return _maxVoltage; }
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
    /// @brief The pin number of the motor input A.
    PinNumber _motorInputA = -1;
    /// @brief The pin number of the motor input B.
    PinNumber _motorInputB = -1;
    /// @brief The minimum voltage value that represents 100% voltage drop of the potentiometer.
    Volts _minVoltage = 0.0f;
    /// @brief The maximum voltage value that represents 0% voltage drop of the potentiometer.
    Volts _maxVoltage = 0.0f;
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
    ErrorType calibrateMinimumForwardSlideTime(Milliseconds &minimumForwardSlideTime, const Volts hysteresis);
    /**
     * @brief Determine the minimum number of milliseconds needed to move the wiper backward at the current speed.
     * @param[out] minimumBackwardSlideTime The minimum amount of time needed to slide forward.
     * @param[in] hysteresis The minimum acceptable voltage to count as a change.
     * @returns ErrorType::Success if the calibration was successfull
     * @returns ErrorType::Failure otherwise.
     */
    ErrorType calibrateMinimumBackwardSlideTime(Milliseconds &minimumBackwardSlideTime, const Volts hysteresis);
};

#endif //__SM10001_HPP__
