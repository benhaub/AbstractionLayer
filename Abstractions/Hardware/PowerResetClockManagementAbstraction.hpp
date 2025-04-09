/***************************************************************************//**
* @author   Ben Haubrich
* @file     PowerResetClockManagementAbstraction.hpp
* @details  Abstraction layer for power, reset and clock management
* @ingroup  Abstractions
*******************************************************************************/
#ifndef __POWER_RESET_CLOCK_MANAGEMENT_HPP__
#define __POWER_RESET_CLOCK_MANAGEMENT_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"

/**
 * @class PowerResetClockManagementAbstraction
 * @brief Abstraction layer for power, reset, and clock management
 */
class PowerResetClockManagementAbstraction {
    public:
    ///@brief constructor
    PowerResetClockManagementAbstraction() = default;
    ///@brief destructor
    virtual ~PowerResetClockManagementAbstraction() = default;

    /**
     * @brief Initialize the power, reset, and clock management
     * @returns ErrorType::Success if the power, reset, and clock management was initialized successfully
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType init() = 0;

    /**
     * @brief Set the CPU clock frequency
     * @param frequency The desired clock frequency
     * @param externalCrystalFrequency The frequency of the external crystal.
     * @returns ErrorType::Success if the clock frequency was set successfully
     * @returns ErrorType::Failure otherwise
     * @post If the system does not support desired frequency, the closest supported frequency will be set
     */
    virtual ErrorType setClockFrequency(const Hertz frequency, const Hertz externalCrystalFrequency) = 0;
};

#endif // __POWER_RESET_CLOCK_MANAGEMENT_HPP__