/***************************************************************************//**
* @author   Ben Haubrich
* @file     PwmAbstraction.hpp
* @details  Abstraction for PWM
* @ingroup  Abstractions
*******************************************************************************/
#ifndef __PWM_ABSTRACTION_HPP__
#define __PWM_ABSTRACTION_HPP__

#include "Error.hpp"
#include "Types.hpp"

/**
 * @class PwmAbstraction
 * @brief Abstraction layer for PWM
 */
class PwmAbstraction {

    public:
    ///@brief Constructor
    PwmAbstraction() = default;
    ///@brief Destructor
    virtual ~PwmAbstraction() = default;

    /// @brief The tag for logging
    static constexpr char TAG[] = "StandalonePwm";

    /**
     * @brief Initialize the PWM driver
     * @returns ErrorType::Success if the driver was initialized
     * @returns ErrorType::Failure if the driver was not initialized
     */
    virtual ErrorType init() = 0;
    /**
     * @brief Deinitialize the PWM driver
     * @returns ErrorType::Success if the driver was deinitialized
     * @returns ErrorType::Failure if the driver was not deinitialized
     */
    virtual ErrorType deinit() = 0;
    /**
     * @brief start the signal
     * @returns ErrorType::Success if the signal was started
     * @returns ErrorType::Failure if the driver was not started
     */
    virtual ErrorType start() = 0;
    /**
     * @brief stop the signal
     * @returns ErrorType::Success if the signal was stopped
     * @returns ErrorType::Failure if the signal was not stopped
     */
    virtual ErrorType stop() = 0;
    /**
     * @brief Set the percentage of time within the period that the signal remains high.
     * @param[in] on The percentage of time the signal is on within the period.
     * @note The average voltage output by the PWM is the logic high voltage of the pin multiplied
     *       by the duty cycle.
     * @returns ErrorType::Success if the duty cycle was set
     * @returns ErrorType::Failure if the duty cycle was not set
     */
    virtual ErrorType setDutyCycle(Percent on) = 0;
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

    /// @brief Get the output pin as a const reference
    const PinNumber &outputPinConst() const { return _outputPin; }
    /**
     * @brief Get the output pin as a mutable reference
     * @note The output pin is set like this because some systems may include pin muxing tools that will set the pins for you
     *       in the driver config so setting the pin here may not always be required.
     */
    PinNumber &outputPin() { return _outputPin; }
    /// @brief Get the duty cycle as a mutable reference
    Percent &dutyCycle() { return _dutyCycle; }
    /// @brief Get the duty cycle as a const reference
    const Percent &dutyCycleConst() const { return _dutyCycle; }
    /// @brief Get the period as a mutable reference
    Microseconds &period() { return _period; }
    /// @brief Get the period as a const reference
    const Microseconds &periodConst() const { return _period; }
    /// @brief Get the peripheral number as a mutable reference
    PeripheralNumber &peripheralNumber() { return _peripheral; }
    /// @brief Get the peripheral number as a const reference
    const PeripheralNumber &peripheralNumberConst() const { return _peripheral; }

    protected:
    /// @brief The duty for the PWM
    Percent _dutyCycle = 0;
    /// @brief The time to apply the duty cyle to.
    Microseconds _period = 0;
    /// @brief The peripheral number for this PWM
    PeripheralNumber _peripheral = PeripheralNumber::Unknown;
    /// @brief The output pin
    PinNumber _outputPin = 0;
};

#endif //__PWM_ABSTRACTION_HPP__