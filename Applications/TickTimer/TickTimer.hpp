
/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   TickTimer.hpp
* @details A lighweight system tick timer.
* @ingroup Applications
*******************************************************************************/
#ifndef __TICK_TIMER_HPP__
#define __TICK_TIMER_HPP__

//AbstractionLayer
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
     * @param startTime The time at which the timer started
     * @param duration The duration for which the timer should run
     * @returns ErrorType::Success if the difference between the current timer and the provided start time has been reached
     * @returns ErrorType::Negative If the duration between the current timer and the provided start time has not been reached.
     * @returns Anything that is returned by OperatingSystem::getSystemTime
     * @code{.cpp}
     * Ticks delay = 1000;
     * ErrorType error = ErrorType::Negative;

     * if (delay >= 1000) {
     *     error = this->delay(Milliseconds(delay / 1000));
     * }
     * else {
     *     Ticks toWait;
     *     millisecondsToTicks(delay/1000, toWait);
     *     const Ticks startTime = xTaskGetTickCount();

     *     while (ErrorType::Success != error) {
     *         error = TickTimer::Timer(startTime, toWait);
     *     }
     * }
     * @endcode
     */
    ErrorType Timer(const Ticks startTime, const Ticks duration) {
        const Ticks timeoutTime = startTime + duration;

        Ticks rolloverCompensation = 0;
        if (timeoutTime < startTime) {
            //Plus one since the roll-over to zero counts as a tick.
            rolloverCompensation =  std::numeric_limits<Ticks>::max() - startTime + 1;
        }

        Ticks currentTime;
        OperatingSystem::Instance().getSystemTick(currentTime);

        if (currentTime + rolloverCompensation < timeoutTime) {
            return ErrorType::Negative;
        }
        else {
            return ErrorType::Success;
        }
    }
}

#endif // __TICK_TIMER_HPP__