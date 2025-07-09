/***************************************************************************//**
* @author  Ben Haubrich
* @file    HBridgeAbstraction.hpp
* @details Abstraction for H-Bridge
* @ingroup Abstractions
*******************************************************************************/
#ifndef __HBRIDGE_ABSTRACTION_HPP__
#define __HBRIDGE_ABSTRACTION_HPP__

//AbstractionLayer
#include "GptmPwmAbstraction.hpp"
#include "PwmAbstraction.hpp"
#include "GpioAbstraction.hpp"
//C++
#include <array>
#include <memory>

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

    /**
     * @brief Set the PWMs.
     * @param gptPwms The PWMs implemented by the general purpose timer.
     */
    void setPwms(std::array<std::unique_ptr<GptmPwmAbstraction>, 2> &gptPwms) {
        _isDrivenByGptmPwm = true;
        _gptPwms.swap(gptPwms);
    }

    /**
     * @brief Set the PWMs.
     * @param pwms The PWMs implemented by a standalone driver.
     */
    void setPwms(std::array<std::unique_ptr<PwmAbstraction>, 2> &pwms) {
        _isDrivenByStandalonePwm = true;
        _pwms.swap(pwms);
    }

    /**
     * @brief Set the GPIOs.
     * @param gpios The GPIOs used to drive the H-Bridge.
     */
    void setGpios(std::array<std::unique_ptr<GpioAbstraction>, 2> &gpios) {
        _isDrivenByGpio = true;
        _gpios.swap(gpios);
    }

    /// @brief Get a mutable reference to the pwms
    std::array<std::unique_ptr<GptmPwmAbstraction>, 2> &gptPwms() { return _gptPwms; }
    /// @brief Get a constant reference to the pwms.
    const std::array<std::unique_ptr<GptmPwmAbstraction>, 2> &gptPwmsConst() const { return _gptPwms; }
    /// @brief Get a mutable reference to the pwms
    std::array<std::unique_ptr<PwmAbstraction>, 2> &standalonePwms() { return _pwms; }
    /// @brief Get a constant reference to the pwms.
    const std::array<std::unique_ptr<PwmAbstraction>, 2> &standalonePwmsConst() const { return _pwms; }
    /// @brief Get a mutable reference to the gpios
    std::array<std::unique_ptr<GpioAbstraction>, 2> &gpios() { return _gpios; }
    /// @brief Get a constant reference to the gpios.
    const std::array<std::unique_ptr<GpioAbstraction>, 2> &gpiosConst() const { return _gpios; }
    /// @brief true if driven by GptmPwm
    bool isDrivenByGptmPwm() const { return _isDrivenByGptmPwm; }
    /// @brief true if driven by standalone Pwm
    bool isDrivenByStandalonePwm() const { return _isDrivenByStandalonePwm; }
    /// @brief true if driven by GPIO
    bool isDrivenByGpio() const { return _isDrivenByGpio; }

    protected:
    /// @brief The PWMs implemented by the general purpose timer.
    std::array<std::unique_ptr<GptmPwmAbstraction>, 2> _gptPwms;
    /// @brief The PWMs implemented by a standalone driver.
    std::array<std::unique_ptr<PwmAbstraction>, 2> _pwms;
    /// @brief The GPIOs that are used to drive the H-Bridge.
    std::array<std::unique_ptr<GpioAbstraction>, 2> _gpios;

    private:
    /// @brief True for systems that implement PWM by the general purpose timer.
    bool _isDrivenByGptmPwm = false;
    /// @brief True for systems that implement PWM by a standalone driver.
    bool _isDrivenByStandalonePwm = false;
    /// @brief True for systems that implement H-Bridge by GPIOs.
    bool _isDrivenByGpio = false;
};

#endif //__HBRIDGE_ABSTRACTION_HPP__