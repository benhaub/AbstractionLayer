/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   Types.hpp
* @details \b Synopsis: \n Variable types for enhanced readability.
* @ingroup Utilities
*******************************************************************************/

#ifndef __TYPES_HPP__
#define __TYPES_HPP__

//C++
#include <cstdint>
#include <cassert>

//-------------------------------Time
///@typedef Microseconds
///Microseconds (μs)
using Microseconds = uint64_t;
///@typedef Milliseconds
///Milliseconds (ms)
using Milliseconds = uint32_t;
///@typedef Seconds
///Seconds (s)
using Seconds = uint32_t;
///@typedef Minutes
///Minutes
using Minutes = uint32_t;
///@typedef Hours
///Hours (h)
using Hours = uint32_t;
///@typedef UnixTime
///Unix time (s)
using UnixTime = uint32_t;
///@typedef Ticks
///CPU tick. Must use a system dependant conversion to get elapsed time in seconds.
using Ticks = uint32_t;
///@struct DateTime
///@brief Date and time
struct DateTime {
    uint8_t second; ///< Seconds. 0-59
    uint8_t minute; ///< minutes. 0-59
    uint8_t hour;   ///< hour. 0-23
    uint8_t day;    ///< day. 1-31
    uint8_t weekday;///< week 1-7 (Sun-Sat)
    uint8_t month;  ///< month. 1-12
    uint16_t year;   ///< year.

    bool operator == (const DateTime &other) const {
        return year == other.year && month == other.month && day == other.day &&
               hour == other.hour && minute == other.minute && second == other.second;
    }
    bool operator < (const DateTime &other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        if (day != other.day) return day < other.day;
        if (hour != other.hour) return hour < other.hour;
        if (minute != other.minute) return minute < other.minute;
        return second < other.second;
    }
    bool operator > (const DateTime &other) const {
        if (year != other.year) return year > other.year;
        if (month != other.month) return month > other.month;
        if (day != other.day) return day > other.day;
        if (hour != other.hour) return hour > other.hour;
        if (minute != other.minute) return minute > other.minute;
        return second > other.second;
    }
    bool operator <= (const DateTime &other) const {
        return *this < other || *this == other;
    }
    bool operator >= (const DateTime &other) const {
        return *this > other || *this == other;
    }
    DateTime operator = (const DateTime &other) {
        second = other.second;
        minute = other.minute;
        hour = other.hour;
        day = other.day;
        weekday = other.weekday;
        month = other.month;
        year = other.year;
        return *this;
    }
};
///@struct Time
///@brief Time
struct Time {
    uint8_t second; ///< Seconds. 0-59
    uint8_t minute; ///< minutes. 0-59
    uint8_t hour;   ///< hour. 0-23
};
///@struct Date
///@brief Date
struct Date {
    uint8_t day;    ///< day. 1-31
    uint8_t weekday;///< week 1-7 (Sun-Sat)
    uint8_t month;  ///< month. 1-12
    uint16_t year;  ///< year.
};

//-------------------------------Storage sizes
///@typedef Bytes
///Bytes (B)
using Bytes = uint32_t;
///@typedef Kilobytes
///Kilobytes (kiB)
using Kilobytes = uint32_t;
///@typedef Megabytes
///Megabytes (MiB)
using Megabytes = uint32_t;
///@typedef FileOffset
///File offset (B)
using FileOffset = uint32_t;

//-------------------------------Physical quantities
///@typedef DecibelMilliWatts
///Decibel milliwatts (dBm)
using DecibelMilliWatts = int16_t;
///@typedef Celcius
///Celcius (C)
using Celcius = float;
///@typedef Volts
///Volts (V)
using Volts = float;
///@typedef Ampere
///Amps (A)
using Ampere = float;
///@typedef Hertz
///Assuming all freqencies will be less than or equal to a second.
///Hertz (Hz) (1/s)
using Hertz = uint32_t;

//-------------------------------Number representations
///@typedef Percent
///Percentage (%)
using Percent = float;

//-------------------------------Algorithms
///@typedef Count
///@code Count negativeAcks @endcode
///@code Count transmissions @endcode
///@code Count receivedMessages @endcode
using Count = uint32_t;
///@typedef Id
///Identification number
using Id = uint32_t;

//-------------------------------Drivers and low level code
///@typedef PartitionNumber
///Partition number
using PartitionNumber = uint16_t;
///@typedef PinNumber
///Pin number. Signed so that - can be used to represent unused or not connected pins.
using PinNumber = int32_t;
/**
 * @enum PeripheralNumber
 * @brief Hardware peripheral number. Refers to the number or index given to the hardware peripheral being used.
 * @details This is an enum because PeripheralNumber do not change at runtime and should be set statically.
*/
enum class PeripheralNumber : uint8_t {
    Zero = 0, ///< Peripheral number 0.
    One,      ///< Peripheral number 1.
    Two,      ///< Peripheral number 2.
    Three,    ///< Peripheral number 3.
    Four,     ///< Peripheral number 4.
    Five,     ///< Peripheral number 5.
    Six,      ///< Peripheral number 6.
    Seven,    ///< Peripheral number 7.
    Eight,    ///< Peripheral number 8.
    Unknown   ///< Unknown and invalid peripheral number.
};
///@typedef Register
///Hardware register address
using Register = volatile uint32_t *;

//-------------------------------Networks
///@typedef Port
///Network port number
using Port = uint16_t;
///@typedef Socket
///Network socket number
using Socket = int32_t;
///@typedef Ipv4Address
///IP version 4 address
using Ipv4Address = uint32_t;

/**
 * @brief Converts a DateTime structure to a Unix time.
 * @param dt The DateTime structure to convert.
 * @return The Unix time equivalent of the DateTime.
 * @note Remember the datetime data should represent the time and date since January 1st, 1970. If you provide a datetime
 *       with the year 2025 you are asking for the date which is 2025 years since 1970.
 * @sa Adapted from Freescale Semiconductor fsl_snvs_hp.c
 */
static constexpr UnixTime ToUnixTime(DateTime dt) {
    constexpr uint16_t daysInAYear = 365;
    constexpr uint32_t secondsInADay = 86400;
    constexpr uint16_t secondsInAnHour = 3600;
    constexpr uint16_t secondsInAMinute = 60;

    //Number of days from the beginning of the non leap-year
    auto daysFromJanuaryToNow = [](uint8_t month) {
        if (month > 12 || month < 1) {
            return 0;
        }

        switch (month) {
            case 1:
            case 2:
                return 0;
            case 3:
                return 31;
            case 4:
                return 90;
            case 5:
                return 120;
            case 6:
                return 151;
            case 7:
                return 181;
            case 8:
                return 212;
            case 9:
                return 243;
            case 10:
                return 273;
            case 11:
                return 304;
            case 12:
                return 334;
            default:
                return 0;
        }

        return 0;
    };

    //Since the epoch from the given year
    uint16_t daysSince = dt.year * daysInAYear;
    //Since the epoch of the given year with leap days included.
    daysSince += ((1970 + dt.year) / 4) - (1970 / 4);
    //We are at January of this year, now go to our month.
    daysSince += daysFromJanuaryToNow(dt.month);
    //Now go to the day for the month.
    if (dt.day > 0) {
        daysSince += ((uint32_t )dt.day - 1);
    }
    //If this year is a leap year and it's less than or equal February
    if (0 != dt.year && 0 == (dt.year & 3) && dt.year <= 2) {
        daysSince--;
    }

    uint32_t secondsSince = (daysSince * secondsInADay) + (dt.hour * secondsInAnHour) + (dt.minute * secondsInAMinute) + dt.second;

    return secondsSince;
}

/**
 * @brief Convert a Unix time to a date time
 * @param seconds The unix time
 * @returns the DateTime which is the time and date passed since January 1st, 1970.
 */
static constexpr DateTime ToDateTime(UnixTime seconds) {
    constexpr uint16_t daysInAYear = 365;
    constexpr uint32_t secondsInADay = 86400;
    constexpr uint16_t secondsInAnHour = 3600;
    constexpr uint16_t secondsInAMinute = 60;

    auto daysPerMonth = [](uint16_t currentYear, uint8_t month) {
        if (month > 12 || month < 1) {
            return 0;
        }

        if (0 != (currentYear & 3) && month == 2) {
            return 29;
        } 

        switch (month) {
            case 1:
                return 31;
            case 2:
                return 28;
            case 3:
                return 31;
            case 4:
                return 30;
            case 5:
                return 31;
            case 6:
                return 30;
            case 7:
                return 31;
            case 8:
                return 31;
            case 9:
                return 30;
            case 10:
                return 31;
            case 11:
                return 30;
            case 12:
                return 31;
            default:
                return 0;
        }

        return 0;
    };

    uint32_t secondsLeftToAddToDatetime = seconds;
    uint16_t daysSince = seconds / secondsInADay;
    secondsLeftToAddToDatetime %= secondsInADay;

    DateTime dateSince {};
    dateSince.hour = (uint8_t)(secondsLeftToAddToDatetime / secondsInAnHour);
    secondsLeftToAddToDatetime %= secondsInAnHour;
    dateSince.minute = (uint8_t)(secondsLeftToAddToDatetime / secondsInAMinute);
    dateSince.second = (uint8_t)(secondsLeftToAddToDatetime % secondsInAMinute);
    uint16_t yearsSinceStartingFrom1970 = 0;

    while (daysSince > daysInAYear) {
        if ((dateSince.year & 3U) == 0U) {
            daysSince--;
        }

        daysSince -= daysInAYear;
        yearsSinceStartingFrom1970++;
    }

    dateSince.year = yearsSinceStartingFrom1970;

    for (int m = 0; m <= 12 && daysSince > 0; m++) {
        uint8_t daysInMonth = daysPerMonth(dateSince.year, m);
        if (0 != daysInMonth && daysSince <= daysInMonth) {
            dateSince.month = m;
        }
        else {
            daysSince -= daysInMonth;
        }
    }

    dateSince.day = daysSince;

    return dateSince;
}
/**
 * @brief Converts an IPv4 address to its first octet
 * @param address The IPv4 address to convert
 * @return The first octet of the IPv4 address (123.xxx.xxx.xxx)
 */
constexpr inline uint8_t ipv4AddressToOctet1(const Ipv4Address address) {
    return (address >> 24) & 0xFF;
}
/**
 * @brief Converts an IPv4 address to its second octet
 * @param address The IPv4 address to convert
 * @return The second octet of the IPv4 address (xxx.xxx.123.xxx)
 */
constexpr inline uint8_t ipv4AddressToOctet2(const Ipv4Address address) {
    return (address >> 16) & 0xFF;
}
/**
 * @brief Converts an IPv4 address to its third octet
 * @param address The IPv4 address to convert
 * @return The third octet of the IPv4 address (xxx.xxx.123.xxx)
 */
constexpr inline uint8_t ipv4AddressToOctet3(const Ipv4Address address) {
    return (address >> 8) & 0xFF;
}
/**
 * @brief Converts an IPv4 address to its fourth octet
 * @param address The IPv4 address to convert
 * @return The fourth octet of the IPv4 address (xxx.xxx.xxx.123)
 */
constexpr inline uint8_t ipv4AddressToOctet4(const Ipv4Address address) {
    return (address) & 0xFF;
}

#endif //__TYPES_HPP__