/***************************************************************************//**
* @author   Ben Haubrich
* @file     PowerResetClockManagementAbstraction.hpp
* @details  Abstraction layer for power and clock management
* @ingroup  AbstractionLayer
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

    virtual ErrorType init() = 0;
};

#endif // __POWER_RESET_CLOCK_MANAGEMENT_HPP__