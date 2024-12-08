/***************************************************************************//**
* @author   Ben Haubrich
* @file     PwmAbstraction.hpp
* @details  Abstraction layer for PWM
* @ingroup  AbstractionLayer
*******************************************************************************/
#ifndef __PWM_ABSTRACTION_HPP__
#define __PWM_ABSTRACTION_HPP__

#include "Error.hpp"
#include "Types.hpp"

namespace PwmConfig {

    enum class PeripheralNumber : uint8_t {
        Zero = 0,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Unknown
    };
}

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

    /**
     * @brief Initialize the PWM driver
     * @returns ErrorType::Success if the driver was initialized
     * @returns ErrorType::Failure if the driver was not initialized
     */
    virtual ErrorType init() = 0;
    /**
     * @brief Set parameters related to the hardware
     * @returns ErrorType::Success if the config was set
     * @returns ErrorType::InvalidParameter if the parameters are invalid
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType setHardwareConfig(PwmConfig::PeripheralNumber peripheral) = 0;
    /**
     * @brief Set parameters related to the driver
     * @returns ErrorType::Success if the config was set
     * @returns ErrorType::InvalidParameter if the parameters are invalid
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType setDriverConfig(Percent duty, Hertz Frequency) = 0;
    /**
     * @brief Set parameters related to the firmware's interaction with the driver
     * @returns ErrorType::Success if the config was set
     * @returns ErrorType::InvalidParameter if the parameters are invalid
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType setFirmwareConfig() = 0;
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
     * @brief Set the frequency at which the signal switches from high to low
     * @param[in] frequency The frequency at which the signal switches
     * @note If the frequency is adjusted and the duty cycle stays the same then
     *       the signal will switch less and remain high for longer.
     *       Typically, frequency is fixed for a given application while the duty cycle
     *       is adjusted.
     * @returns ErrorType::Success if the frequency was set.
     * @returns ErrorType::Failure if the frequency was not set.
     */
    virtual ErrorType setFrequency(Hertz frequency) = 0;

    Percent &dutyCycle() { return _dutyCycle; }
    const Percent &dutyCycleConst() const { return _dutyCycle; }
    Hertz &frequency() { return _frequency; }
    const Hertz &frequencyConst() const { return _frequency; }
    PwmConfig::PeripheralNumber &peripheralNumber() { return _peripheral; }
    const PwmConfig::PeripheralNumber &peripheralNumberConst() const { return _peripheral; }

    protected:
    /// @brief The duty for the PWM
    Percent _dutyCycle = 0;
    /// @brief The frequency at which the PWM will switch high to low
    Hertz _frequency = 0;
    /// @brief The peripheral number for this PWM
    PwmConfig::PeripheralNumber _peripheral = PwmConfig::PeripheralNumber::Unknown;
};

#endif //__PWM_ABSTRACTION_HPP__