/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   Sm10001.hpp
* @details \b Synopsis: \n Driver for the slide potentiometer by adafruit
* @see https://www.adafruit.com/product/5466
* @ingroup Applications
*******************************************************************************/
#ifndef __SM10001_HPP__
#define __SM10001_HPP__

//AbstractionLayer
#include "AdcAbstraction.hpp"
#include "HBridgeAbstraction.hpp"
//C++
#include <vector>
#include <memory>

/**
 * @class Sm10001
 * @brief Driver for the slide potentiometer by adafruit
 */
class Sm10001 {

    public:
    /**
     * @brief Constructor
     * @param hBridge The HBridge to use
     * @param adc The ADC to use
     * @param motorInputA The pin number of the motor input A.
     * @param motorInputB The pin number of the motor input B.
     * @post Ownership of the HBridge is transferred to the SM10001.
     * @post Ownership of the ADC is taken by this SM10001
     */
    Sm10001(std::unique_ptr<HBridgeAbstraction> &hBridge,
            std::unique_ptr<AdcAbstraction> &adc,
            PinNumber motorInputA, PinNumber motorInputB) {
        _hBridge = std::move(hBridge);
        _adc = std::move(adc);
        _motorInputA = motorInputA;
        _motorInputB = motorInputB;

        //Note that this only begins to calculate once the period falls below 1000.
        static_assert(1000 / _PwmPeriod <= 250, "Frequency is beyond max reccommended in data sheet.");
    }
    /// @brief Destructor
    ~Sm10001() = default;

    /// @brief Number of input pins on the motor.
    static constexpr Count _MotorInputPins = 2;
    /// @brief Period of the PWMs for the motor.
    static constexpr Microseconds _PwmPeriod = 1000;
    /// @brief Duty cycle of the PWMs for the motor.
    static constexpr Percent _PwmDutyCycle = 100;

    /**
     * @brief init the PWMs for the motor
     * @returns ErrorType::Success if the PWMs were initialized
     * @returns ErrorType::Failure otherwise
     */
    ErrorType init();
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

    private:
    /// @brief The H-Bridge that drives the motor
    std::unique_ptr<HBridgeAbstraction> _hBridge;
    /// @brief The ADC that reads the potentiometer voltage drop.
    std::unique_ptr<AdcAbstraction> _adc;
    /// @brief The pin number of the motor input A.
    PinNumber _motorInputA;
    /// @brief The pin number of the motor input B.
    PinNumber _motorInputB;
};

#endif //__SM10001_HPP__
