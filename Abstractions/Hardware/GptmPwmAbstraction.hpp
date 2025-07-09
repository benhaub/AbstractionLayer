/***************************************************************************//**
* @author  Ben Haubrich
* @file    GptmPwmAbstraction.hpp
* @details Abstraction for GPTM PWM
* @ingroup Abstractions
*******************************************************************************/
#ifndef __GPTM_PWM_ABSTRACTION_HPP__
#define __GPTM_PWM_ABSTRACTION_HPP__

//AbstractionLayer
#include "GptmAbstraction.hpp"

/**
 * @class GptmPwmAbstraction
 * @brief Abstraction layer for GPTM PWM
*/
class GptmPwmAbstraction : public GptmAbstraction {

    public:
    ///@brief Constructor
    GptmPwmAbstraction() : GptmAbstraction() {}
    ///@brief Destructor
    virtual ~GptmPwmAbstraction() = default;

    /**
     * @brief Set the percentage of time within the period that the signal remains high.
     * @param[in] on The percentage of time the signal is on within the period.
     * @note The average voltage output by the PWM is the logic high voltage of the pin multiplied
     *       by the duty cycle.
     * @returns ErrorType::Success if the duty cycle was set
     * @returns ErrorType::Failure if the duty cycle was not set
     */
    virtual ErrorType setDutyCycle(const Percent on) = 0;
    /**
     * @brief Set the period during which the signal switches from high to low
     * @param[in] period The period during which the signal switches
     * @note If the period is adjusted to be longer and the duty cycle stays the same then
     *       the signal will switch less and remain high for longer.
     *       Typically, period is fixed for a given application while the duty cycle
     *       is adjusted.
     * @returns ErrorType::Success if the period was set.
     * @returns ErrorType::Failure if the period was not set.
     */
    virtual ErrorType setPeriod(const Microseconds period) = 0;

    /// @brief Get the duty cycle as a const reference
    const Percent &dutyCycleConst() const { return _dutyCycle; }
    /// @brief Get the period as a const reference
    const Microseconds &periodConst() const { return _period; }
    /// @brief Get the output pin as a const reference
    const PinNumber &outputPinConst() const { return _outputPin; }
    /**
     * @brief Get the output pin as a mutable reference
     * @note The output pin is set like this because some systems may include pin muxing tools that will set the pins for you
     *       in the driver config so setting the pin here may not always be required.
     */
    PinNumber &outputPin() { return _outputPin; }

    protected:
    /// @brief The duty for the PWM
    Percent _dutyCycle = 0;
    /// @brief The time to apply the duty cyle to.
    Microseconds _period = 0;
    /// @brief The output pin
    PinNumber _outputPin = 0;
};

#endif //__GPTM_PWM_ABSTRACTION_HPP__