/***************************************************************************//**
* @author  Ben Haubrich
* @file    DarlingtonArrayAbstraction.hpp
* @details Abstraction for Darlington Array
* @ingroup Abstractions
*******************************************************************************/
#ifndef __DARLINTON_ARRAY_ABSTRACTION_HPP__
#define __DARLINTON_ARRAY_ABSTRACTION_HPP__

//AbstractionLayer
#include "GptmPwmModule.hpp"
#include "PwmModule.hpp"
#include "GpioModule.hpp"
//C++
#include <array>
#include <memory>

namespace DarlingtonArrayTypes {

    /**
     * @enum PwmType
     * @brief The type of PWM used to drive the H-Bridge.
     */
    enum class PwmType : uint8_t {
        Unknown = 0, ///< Unknown PWM type
        Gptm,        ///< PWM implemented by the general purpose timer
        Gpio,        ///< PWM implemented by GPIO pins
        Standalone   ///< Dedicated PWM hardware.
    };

    /**
     * @struct Params
     * @tparam numberOfPins The number of pins that the transistor array has.
     * @brief Parameters for the DarlingtonArray
     */
    template <Count _numberOfPins>
    struct ConfigurationParameters {
        DarlingtonArrayTypes::PwmType pwmType; ///< The type of input used.
        std::array<PinNumber, _numberOfPins> pinNumbers; ///< The pin numbers that are connected to the inputs of the array.
        std::array<PeripheralNumber, _numberOfPins> pwmPeripherals; ///< The peripheral number of pwm1.
        std::array<Microseconds, _numberOfPins> pwmPeriods; ///< The period of the PWM signals.
        std::array<Percent, _numberOfPins> pwmDutyCycles; ///< The duty cycle of the PWM signals.
        Count numberOfPinsUsed;

        ConfigurationParameters() {
            pwmType = DarlingtonArrayTypes::PwmType::Unknown;
            pinNumbers.fill(-1);
            pwmPeripherals.fill(PeripheralNumber::Unknown);
            pwmPeriods.fill(0);
            pwmDutyCycles.fill(0.0f);
            numberOfPinsUsed = 0;
        }
    };
}

/**
 * @class DarlingtonArrayAbstraction
 * @tparam _numberOfPins The number of pins on the Darlington Array
 * @brief The Darlington Array that drives the motor.
 */
template <Count _numberOfPins>
class DarlingtonArrayAbstraction {
    public:
    /// @brief Constructor
    DarlingtonArrayAbstraction() = default;
    /// @brief Destructor
    virtual ~DarlingtonArrayAbstraction() = default;

    /**
     * @brief Configure the Darlington Array.
     * @param pwmType The type of PWM used to drive the array.
     * @returns ErrorType::Success if the array was configured
     * @returns ErrorType::Failure if the array was not configured
     */
    virtual ErrorType configure(const DarlingtonArrayTypes::ConfigurationParameters<_numberOfPins> &params) {
        _params = params;
        return ErrorType::Success;
    }

    /**
     * @brief Initialize the darlington array
     * @pre configure
     * @returns ErrorType::Success if the darlington array was configured.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType init() = 0;
    /**
     * @brief toggle a pin
     * @param pinNumber The pin to toggle
     * @param on true to turn the pin on, false to turn it off
     * @returns ErrorType::Success if the pin was toggled
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType togglePin(Count pinNumber, bool on) = 0;

    /// @brief true if driven by GptmPwm
    bool isDrivenByGptmPwm() const { return DarlingtonArrayTypes::PwmType::Gptm == _params.pwmType; }
    /// @brief true if driven by standalone Pwm
    bool isDrivenByStandalonePwm() const { return DarlingtonArrayTypes::PwmType::Standalone == _params.pwmType; }
    /// @brief true if driven by GPIO
    bool isDrivenByGpio() const { return DarlingtonArrayTypes::PwmType::Gpio == _params.pwmType; }
    /// @brief Get the configuration parameters as a constant reference
    const DarlingtonArrayTypes::ConfigurationParameters<_numberOfPins> &params() const { return _params; }
    /// @brief Get the number of input pins available
    constexpr Count inputPins() const { return _numberOfPins; }

    protected:
    /// @brief The PWMs implemented by the general purpose timer.
    std::array<GptmPwmModule, _numberOfPins> _gptPwms;
    /// @brief The PWMs implemented by a standalone driver.
    std::array<Pwm, _numberOfPins> _pwms;
    /// @brief The GPIOs that are used to drive the H-Bridge.
    std::array<Gpio, _numberOfPins> _gpios;

    private:
    DarlingtonArrayTypes::ConfigurationParameters<_numberOfPins> _params;
};

#endif // __DARLINTON_ARRAY_ABSTRACTION_HPP__