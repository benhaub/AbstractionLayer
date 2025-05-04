/***************************************************************************//**
* @author  Ben Haubrich
* @file    DarlingtonArrayAbstraction.hpp
* @details Abstraction for Darlington Array
* @ingroup Abstractions
*******************************************************************************/
#ifndef __DARLINTON_ARRAY_ABSTRACTION_HPP__
#define __DARLINTON_ARRAY_ABSTRACTION_HPP__

//AbstractionLayer
#include "GptmPwmAbstraction.hpp"
#include "PwmAbstraction.hpp"
#include "GpioAbstraction.hpp"
//C++
#include <array>
#include <memory>

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
     * @brief toggle a pin
     * @param pinNumber The pin to toggle
     * @param on true to turn the pin on, false to turn it off
     * @returns ErrorType::Success if the pin was toggled
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType togglePin(Count pinNumber, bool on) = 0;

    /**
     * @brief Set the PWMs.
     * @param gptPwms The PWMs implemented by the general purpose timer.
     */
    void setPwms(std::array<std::unique_ptr<GptmPwmAbstraction>, _numberOfPins> &gptPwms) {
        _isDrivenByGptmPwm = true;
        _gptPwms.swap(gptPwms);
    }

    /**
     * @brief Set the PWMs.
     * @param pwms The PWMs implemented by a standalone driver.
     */
    void setPwms(std::array<std::unique_ptr<PwmAbstraction>, _numberOfPins> &pwms) {
        _isDrivenByStandalonePwm = true;
        _pwms.swap(pwms);
    }

    /**
     * @brief Set the GPIOs.
     * @param gpios The GPIOs used to drive the H-Bridge.
     */
    void setGpios(std::array<std::unique_ptr<GpioAbstraction>, _numberOfPins> &gpios) {
        _isDrivenByGpio = true;
        _gpios.swap(gpios);
    }

    /// @brief Get a mutable reference to the pwms
    std::array<std::unique_ptr<GptmPwmAbstraction>, _numberOfPins> &gptPwms() { return _gptPwms; }
    /// @brief Get a constant reference to the pwms.
    const std::array<std::unique_ptr<GptmPwmAbstraction>, _numberOfPins> &gptPwmsConst() const { return _gptPwms; }
    /// @brief Get a mutable reference to the pwms
    std::array<std::unique_ptr<PwmAbstraction>, _numberOfPins> &standalonePwms() { return _pwms; }
    /// @brief Get a constant reference to the pwms.
    const std::array<std::unique_ptr<PwmAbstraction>, _numberOfPins> &standalonePwmsConst() const { return _pwms; }
    /// @brief Get a mutable reference to the gpios
    std::array<std::unique_ptr<GpioAbstraction>, _numberOfPins> &gpios() { return _gpios; }
    /// @brief Get a constant reference to the gpios.
    const std::array<std::unique_ptr<GpioAbstraction>, _numberOfPins> &gpiosConst() const { return _gpios; }
    /// @brief true if driven by GptmPwm
    bool isDrivenByGptmPwm() const { return _isDrivenByGptmPwm; }
    /// @brief true if driven by standalone Pwm
    bool isDrivenByStandalonePwm() const { return _isDrivenByStandalonePwm; }
    /// @brief true if driven by GPIO
    bool isDrivenByGpio() const { return _isDrivenByGpio; }

    protected:
    /// @brief The PWMs implemented by the general purpose timer.
    std::array<std::unique_ptr<GptmPwmAbstraction>, _numberOfPins> _gptPwms;
    /// @brief The PWMs implemented by a standalone driver.
    std::array<std::unique_ptr<PwmAbstraction>, _numberOfPins> _pwms;
    /// @brief The GPIOs that are used to drive the H-Bridge.
    std::array<std::unique_ptr<GpioAbstraction>, _numberOfPins> _gpios;

    /// @brief True for systems that implement PWM by the general purpose timer.
    bool _isDrivenByGptmPwm = false;
    /// @brief True for systems that implement PWM by a standalone driver.
    bool _isDrivenByStandalonePwm = false;
    /// @brief True for systems that implement H-Bridge by GPIOs.
    bool _isDrivenByGpio = false;
};

#endif // __DARLINTON_ARRAY_ABSTRACTION_HPP__