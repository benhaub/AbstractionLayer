
/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   TickTimer.hpp
* @details A lighweight system tick timer.
* @ingroup Applications
*******************************************************************************/
#ifndef __TICK_TIMER_HPP__
#define __TICK_TIMER_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"
#include "OperatingSystemModule.hpp"
//C++
#include <limits>

/**
 * @namespace TickTimer
 * @brief A lightweight system tick timer.
 */
namespace TickTimer {

    /**
     * @brief Start the tick timer
     * @returns ErrorType::Success if the difference between the current timer and the provided start time has been reached
     * @returns ErrorType::Negative If the duration between the current timer and the provided start time has not been reached.
     * @returns Anything that is returned by OperatingSystem::getSystemTime
     */
    ErrorType Timer(const Ticks startTime, const Ticks duration) {
        Ticks currentTime;
        OperatingSystem::Instance().getSystemTick(currentTime);
        const Ticks timeoutTime = startTime + duration;

        Ticks rolloverCompensation = 0;
        if (timeoutTime < startTime) {
            //Plus one since the roll-over to zero counts as a tick.
            rolloverCompensation =  std::numeric_limits<Ticks>::max() - startTime + 1;
        }

        if (currentTime + rolloverCompensation < timeoutTime) {
            return ErrorType::Negative;
        }
        else {
            return ErrorType::Success;
        }
    }
}

#endif // __TICK_TIMER_HPP__