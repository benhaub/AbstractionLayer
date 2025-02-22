/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   Math.hpp
* @details \b Synopsis: \n Math functions
* @ingroup Utilities
*******************************************************************************/
#ifndef __MATH_HPP__
#define __MATH_HPP__

//AbstractionLayer
#include "Types.hpp"
#include "Error.hpp"
//C++
#include <limits>

/**
 * @brief Calculate a running average.
 * @pre numValues must be greater than 1
 * @param[in] currentAverage The current average value
 * @param[in] newValue The new value to add to the average
 * @param[in] numValues The number of values in the average
 * @return The new average
*/
template<typename T>
constexpr inline T runningAverage(T currentAverage, T newValue, Count numValues) {
    if (numValues > 2) {
        return (currentAverage * (numValues - 1) + newValue) / numValues;
    }
    else {
        return (currentAverage + newValue) / numValues;
    }
}

/**
 * @brief Get the difference between expectedLargerValue and expectedSmallerValue (expectedLargerValue - expectedSmallerValue)
 * @details Handles wrapping of the values.
 * @param[in] expectedLargerValue The value that should be larger
 * @param[in] expectedSmallerValue The value that should be smaller
 * @return The absolute difference between the two values
 */
template<typename T>
constexpr inline T differenceBetween(T expectedLargerValue, T expectedSmallerValue) {
    const T maxValue = std::numeric_limits<T>::max();

    if (expectedSmallerValue > expectedLargerValue) {
        return (maxValue - expectedLargerValue) + (expectedSmallerValue + 1);
    }
    else {
        return expectedLargerValue - expectedSmallerValue;
    }
}

#endif // __MATH_HPP__
