/***************************************************************************//**
* @author  Ben Haubrich
* @file    HBridgeAbstraction.hpp
* @details Abstraction for H-Bridge
* @ingroup Abstractions
*******************************************************************************/
#ifndef __HBRIDGE_ABSTRACTION_HPP__
#define __HBRIDGE_ABSTRACTION_HPP__

//AbstractionLayer
#include "GptmPwmModule.hpp"
#include "PwmModule.hpp"
#include "GpioModule.hpp"
//C++
#include <array>
#include <memory>

/**
 * @namespace HBridgeTypes
 * @brief Contains types related to the H-Bridge abstraction.
 */
namespace HBridgeTypes {

    constexpr size_t NumberOfInputs = 2; ///< Number of inputs to the H-Bridge

    /**
     * @enum PwmType
     * @brief The type of PWM used to drive the H-Bridge.
     */
    enum class PwmType : uint8_t {
        Unknown = 0, ///< Unknown PWM type
        Gptm,        ///< PWM implemented by the general purpose timer
        Gpio,       ///< PWM implemented by GPIO pins
        Standalone   ///< Dedicated PWM hardware.
    };

    /**
     * @struct ConfigurationParameters 
     * @brief Contains the parameters used to configure the HBridge.
     */
    struct ConfigurationParameters {
        HBridgeTypes::PwmType pwmType = HBridgeTypes::PwmType::Unknown; ///< The type of PWM used to drive the H-Bridge.
        PinNumber input1 = -1; ///< The pin number of the first input to the H-Bridge.
        PinNumber input2 = -1; ///< The pin number of the second input to the H-Bridge.
        PeripheralNumber pwm1 = PeripheralNumber::Unknown; ///< The peripheral number of pwm1.
        PeripheralNumber pwm2 = PeripheralNumber::Unknown; ///< The peripheral number of pwm2.
        Microseconds pwmPeriod = 0; ///< The period of the PWM signals.
        Percent pwmDutyCycle = 0;  ///< The duty cycle of the PWM signals.
    };
}

/**
 * @class HBridgeAbstraction
 * @brief The H-Bridge that drives the motor.
 */
class HBridgeAbstraction {
    public:
    /// @brief Constructor
    HBridgeAbstraction() = default;
    /// @brief Destructor
    virtual ~HBridgeAbstraction() = default;

    /// @brief The tag for logging
    static constexpr char TAG[] = "HBridge";

    /**
     * @brief Configure the H-Bridge.
     * @param pwmType The type of PWM used to drive the H-Bridge.
     * @returns ErrorType::Success if the H-Bridge was configured
     * @returns ErrorType::Failure if the H-Bridge was not configured
     */
    virtual ErrorType configure(const HBridgeTypes::ConfigurationParameters &params) {
        _params = params;
        return ErrorType::Success;
    }
    /**
     * @brief Initialize the H-Bridge
     * @returns ErrorType::Success if the H-Bridge was initialized
     * @returns ErrorType::Failure if the H-Bridge was not initialized
     */
    virtual ErrorType init() = 0;
    /**
     * @brief Drives the motor forward.
     * @returns ErrorType::Success if the motor was driven forward
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType driveForward() = 0;
    /**
     * @brief Drives the motor backward.
     * @returns ErrorType::Success if the motor was driven backward
     * @returns ErrorType::Failure otherwise
    */
    virtual ErrorType driveBackward() = 0;
    /**
     * @brief The H-Bridge is put into coast mode for fast current decay
     * @returns ErrorType::Success if the coast was successful
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType coast() = 0;
    /**
     * @brief The H-Bridge is put into brake mode for slow current decay
     * @returns ErrorType::Success if the brake was successful
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType brake() = 0;

    /// @brief true if driven by GptmPwm
    bool isDrivenByGptmPwm() const { return _params.pwmType == HBridgeTypes::PwmType::Gptm; }
    /// @brief true if driven by standalone Pwm
    bool isDrivenByStandalonePwm() const { return _params.pwmType == HBridgeTypes::PwmType::Standalone; }
    /// @brief true if driven by GPIO
    bool isDrivenByGpio() const { return _params.pwmType == HBridgeTypes::PwmType::Gpio; }
    /// @brief Get a constant reference to the H-Bridge parameters
    const HBridgeTypes::ConfigurationParameters &params() const { return _params; }

    protected:
    /// @brief The PWMs implemented by the general purpose timer.
    std::array<GptmPwmModule, HBridgeTypes::NumberOfInputs> _gptPwms;
    /// @brief The PWMs implemented by a standalone driver.
    std::array<Pwm, HBridgeTypes::NumberOfInputs> _pwms;
    /// @brief The GPIOs that are used to drive the H-Bridge.
    std::array<Gpio, HBridgeTypes::NumberOfInputs> _gpios;

    private:
    /// @brief The H-Bridge parameters.
    HBridgeTypes::ConfigurationParameters _params;
};

#endif //__HBRIDGE_ABSTRACTION_HPP__