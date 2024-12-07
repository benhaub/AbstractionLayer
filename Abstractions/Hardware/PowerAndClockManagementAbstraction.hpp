/***************************************************************************//**
* @author   Ben Haubrich
* @file     RtcAbstraction.hpp
* @details  Abstraction layer for power and clock management
* @ingroup  AbstractionLayer
*******************************************************************************/
#ifndef __POWER_AND_CLOCK_MANAGEMENT_HPP__
#define __POWER_AND_CLOCK_MANAGEMENT_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"

/**
 * @class PowerAndClockManagementAbstraction
 * @brief Abstraction layer for power and clock management
 */
class PowerAndClockManagementAbstraction {
    public:
    ///@brief constructor
    PowerAndClockManagementAbstraction() = default;
    ///@brief destructor
    virtual ~PowerAndClockManagementAbstraction() = default;

    virtual ErrorType init() = 0;
}

#endif // __POWER_AND_CLOCK_MANAGEMENT_HPP__