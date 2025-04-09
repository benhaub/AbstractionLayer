/***************************************************************************//**
* @author  Ben Haubrich
* @file    WatchdogAbstraction.hpp
* @details Abstraction for watchdog timer
* @ingroup Abstractions
*******************************************************************************/
#ifndef __WATCHDOG_ABSTRACTION_HPP__
#define __WATCHDOG_ABSTRACTION_HPP__

//AbstractionLayer
#include "Error.hpp"

/**
 * @class WatchdogAbstraction
 * @brief Abstraction layer for watchdog
*/
class WatchdogAbstraction {

    public:
    /**
     * @brief Constructor
    */
    WatchdogAbstraction() = default;
    /**
     * @brief Destructor
    */
    virtual ~WatchdogAbstraction() = default;

    /**
     * @brief Start petting the watchdog
     * @returns ErrorType::NotImplemented if watchdog is not implemented.
     * @returns ErrorType::NotSupported if watchdog is not supported on the system
    */
    virtual ErrorType startPetting() = 0;

    /**
     * @brief Stop petting the watchdog
     * @returns ErrorType::NotImplemented if watchdog is not implemented.
     * @returns ErrorType::NotSupported if watchdog is not supported on the system
     * @post The system will probably reset
    */
    virtual ErrorType stopPetting() = 0;
    /**
     * @brief Pet the watchdog. Reset the watchdog hardware so that it does not trigger a reset.
     * @returns ErrorType::NotImplemented if watchdog is not implemented.
     * @returns ErrorType::NotSupported if watchdog is not supported on the system
    */
    virtual ErrorType pet() = 0;
};

#endif // __WATCHDOG_ABSTRACTION_HPP__
