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
    /// @brief Number of input pins on the motor.
    constexpr Count _MotorInputPins = 2;
    /// @brief Period of the PWMs for the motor.
    constexpr Microseconds _PwmPeriod = 1000;
    /// @brief Duty cycle of the PWMs for the motor.
    constexpr Percent _PwmDutyCycle = 100;
    /// @brief Room for error when comparing against the voltage drop of the potentiometer
    constexpr Volts _VoltageDropTolerance = 0.1f;

    static_assert(1000 / _PwmPeriod <= 250, "Frequency is beyond max recommended in data sheet.");
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
                   const PinNumber motorInputA, const PinNumber motorInputB, const Volts maxPotentiometerDrop) {

        ErrorType error = HBridgeFactory::Factory(hBridgePartNumber, _hBridge);

        if (ErrorType::Success == error) {
            _maxVoltage = maxPotentiometerDrop;

            std::visit([&](auto &hBridge) {
                HBridgeTypes::ConfigurationParameters params;
                params.pwmType = pwmType;
                params.input1 = _motorInputA = motorInputA;
                params.input2 = _motorInputB = motorInputB;
                params.pwm1 = PeripheralNumber::Zero;
                params.pwm2 = PeripheralNumber::One;
                params.pwmPeriod = Sm10001Types::_PwmPeriod;
                params.pwmDutyCycle = Sm10001Types::_PwmDutyCycle;
                hBridge.configure(params);

                error = hBridge.init();

            }, *_hBridge);

            if (ErrorType::Success == error) {
                AdcTypes::Parameters adcParams;
                adcParams.channel = channel;
                adcParams.peripheralNumber = adcPeripheralNumber;
                error = _adc.configure(adcParams);

                error = _adc.init();
            }
        }

        return error;
    }
    /**
     * @brief Slide forward
     * @returns ErrorType::Success if the slide was successful
     * @returns ErrorType::Failure otherwise
     */
    ErrorType slideForward();
    /**
     * @brief Slide backward
     * @returns ErrorType::Success if the slide was successful
     * @returns ErrorType::Failure otherwise
     */
    ErrorType slideBackward();
    /**
     * @brief Get the voltage drop reading from the potentiometer
     * @param volts The voltage drop reading will be stored here
     * @returns ErrorType::Success if the reading was successful
     * @returns ErrorType::Failure otherwise
     * @post volts is invalid only if ErrorType::Failure is returned.
     */
    ErrorType getVoltageDrop(Volts &volts);
    /**
     * @brief Determine the minimum and maximum voltage values that represent 100% and 0% voltage drop of the potentiometer
     * @details This function will update the minimum and max voltage values set by the constructor.
     * @returns ErrorType::Success if the calibration was successful
     * @returns ErrorType::Failure otherwise
     */
    ErrorType calibrate();

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
};

#endif //__SM10001_HPP__
