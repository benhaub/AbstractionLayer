/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   Math.hpp
* @details Math functions
* @ingroup Utilities
*******************************************************************************/
#ifndef __MATH_HPP__
#define __MATH_HPP__

//AbstractionLayer
#include "Types.hpp"
#include "Error.hpp"
//C++
#include <limits>
#include <cmath>

/**
 * @brief Calculate a running average.
 * @pre numValues must be greater than 1
 * @param[in] currentAverage The current average value
 * @param[in] newValue The new value to add to the average
 * @param[in] numValues The number of values in the average
 * @return The new average
*/
template<typename T>
constexpr inline T runningAverage(const T &currentAverage, const T &newValue, const Count &numValues) {
    if (numValues > 2) {
        return (currentAverage * (numValues - 1) + newValue) / numValues;
    }
    else if (numValues != 0) {
        return (currentAverage + newValue) / numValues;
    }

    return currentAverage;
}

/**
 * @brief Get the difference between expectedLargerValue and expectedSmallerValue (expectedLargerValue - expectedSmallerValue)
 * @details Handles wrapping of the values.
 * @param[in] expectedLargerValue The value that should be larger
 * @param[in] expectedSmallerValue The value that should be smaller
 * @return The absolute difference between the two values. If the expected larger value has overflowed then the overflowed value is added to the difference.
 */
template<typename T>
constexpr inline T differenceBetween(const T &expectedLargerValue, const T &expectedSmallerValue) {
    const T maxValue = std::numeric_limits<T>::max();

    if (expectedSmallerValue > expectedLargerValue) {
        //+1 because wrapping to zero counts as an increment (i.e. maxValue + 1 = 0)
        return (maxValue - expectedLargerValue) + (expectedSmallerValue + 1);
    }
    else {
        return expectedLargerValue - expectedSmallerValue;
    }
}

/**
 * @brief Get the difference between expectedLargerValue and expectedSmallerValue (expectedLargerValue - expectedSmallerValue)
 * @details Handles wrapping of the values.
 * @param[in] expectedLargerValue The value that should be larger
 * @param[in] expectedSmallerValue The value that should be smaller
 * @param[in] maxValue The maximum value allowed.
 * @return The absolute difference between the two values. If the expected larger value has overflowed then the overflowed value is added to the difference.
 */
template<typename T>
constexpr inline T differenceBetween(const T &expectedLargerValue, const T &expectedSmallerValue, const T &maxValue) {
    if (expectedSmallerValue > expectedLargerValue) {
        //+1 because wrapping to zero counts as an increment (i.e. maxValue + 1 = 0)
        return (maxValue - expectedLargerValue) + (expectedSmallerValue + 1);
    }
    else {
        return expectedLargerValue - expectedSmallerValue;
    }
}

/**
 * @brief Test if a value is within a the specified error
 * @param[in] value1 The first value to compare
 * @param[in] value2 The second value to compare
 * @param[in] error The error to use for comparison
 * @returns True if the values are within the specified error
 * @returns False otherwise
 */
template<typename T>
constexpr inline bool withinError(const T &value1, const T &value2, const T &error) {
    return std::abs(value1 - value2) <= error;
}

/**
 * @brief Convert a number of bytes to the number of bits
 * @param[in] numberOfBytes The number of bytes to convert
 * @returns The number of bits
 */
template<typename T>
constexpr inline T ToBits(const T &numberOfBytes) {
    return numberOfBytes * 8;
}

#endif // __MATH_HPP__
