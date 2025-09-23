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
#include <cstdio>
#include <array>

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

//Forward declarations for + operator overload in DateTime.
struct DateTime;
static constexpr UnixTime ToUnixTime(const DateTime &dt);
static constexpr DateTime ToDateTime(const UnixTime &seconds);

///@struct DateTime
///@brief Date and time
struct DateTime {
    uint8_t _second; ///< Seconds. 0-59
    uint8_t _minute; ///< minutes. 0-59
    uint8_t _hour;   ///< hour. 0-23
    uint8_t _day;    ///< day. 1-31
    uint8_t _weekday;///< week 1-7 (Sun-Sat)
    uint8_t _month;  ///< month. 1-12
    uint16_t _year;  ///< years since 1970 (e.g. 1 is 1971, 2 is 1972, etc.).

    /// @brief Constructor. Initializes to UnixTime 0
    constexpr DateTime() : _second(0), _minute(0), _hour(0), _day(1), _weekday(1), _month(1), _year(0) {}
    /// @brief Copy constructor
    constexpr DateTime(const DateTime &other) = default;

    /**
     * @brief Convert the DateTime to a string
     * @return The DateTime as a string
     */
    std::array<char, 21> toString() const {
        std::array<char, 21> string;
        snprintf(string.data(), string.size(), "%u-%u-%u %u:%u:%u", _year, _month, _day, _hour, _minute, _second);
        return string;
    }

    /**
     * @brief Checks if two DateTime objects are equal
     * @param other The DateTime object to compare to
     * @return true if the DateTime objects are equal
     * @return false if the DateTime objects are not equal
     */
    bool operator == (const DateTime &other) const {
        return _year == other._year && _month == other._month && _day == other._day &&
               _hour == other._hour && _minute == other._minute && _second == other._second;
    }
    /**
     * @brief Checks if one Datetime has an earlier data than the other
     * @param other The DateTime object to compare to
     * @return true if this DateTime has an earlier date than the other
     * @return false if this DateTime does not have an earlier date than the other
     */
    bool operator < (const DateTime &other) const {
        if (_year != other._year) return _year < other._year;
        if (_month != other._month) return _month < other._month;
        if (_day != other._day) return _day < other._day;
        if (_hour != other._hour) return _hour < other._hour;
        if (_minute != other._minute) return _minute < other._minute;
        return _second < other._second;
    }
    /**
     * @brief Checks if one Datetime has a later date than the other
     * @param other The DateTime object to compare to
     * @return true if this DateTime has a later date than the other
     * @return false if this DateTime does not have a later date than the other
     */
    bool operator > (const DateTime &other) const {
        if (_year != other._year) return _year > other._year;
        if (_month != other._month) return _month > other._month;
        if (_day != other._day) return _day > other._day;
        if (_hour != other._hour) return _hour > other._hour;
        if (_minute != other._minute) return _minute > other._minute;
        return _second > other._second;
    }
    /**
     * @brief Checks if one Datetime has an earlier date than the other or if they have the same time
     * @param other The DateTime object to compare to
     * @return true if this DateTime has an earlier date than the other or is equal to it
     * @return false if this DateTime does not have an earlier date than the other
     */
    bool operator <= (const DateTime &other) const {
        return *this < other || *this == other;
    }
    /**
     * @brief Checks if one Datetime has a later date than the other or if the have the same time
     * @param other The DateTime object to compare to
     * @return true if this DateTime has a later date than the other or is equal to it
     * @return false if this DateTime does not have a later date than the other
     */
    bool operator >= (const DateTime &other) const {
        return *this > other || *this == other;
    }
    /**
     * @brief Checks if the two dates are the same time
     * @param other The DateTime object to compare to
     * @return true if the two dates are the same time
     * @return false if the two dates are not the same time
     */
    DateTime operator = (const DateTime &other) {
        _second = other._second;
        _minute = other._minute;
        _hour = other._hour;
        _day = other._day;
        _weekday = other._weekday;
        _month = other._month;
        _year = other._year;
        return *this;
    }
    /**
     * @brief Add two DateTime's together
     * @param other The DateTime object to add with
     * @returns The sum of the two DateTime objects
     */
    DateTime operator + (const DateTime &other) {
        UnixTime unixTime = ToUnixTime(*this);
        unixTime += ToUnixTime(other);
        return ToDateTime(unixTime);
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
    uint16_t year;  ///< years since 1970 (e.g. 1 is 1971, 2 is 1972, etc.).
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
    Zero = 0,         ///< Peripheral number 0.
    One,              ///< Peripheral number 1.
    Two,              ///< Peripheral number 2.
    Three,            ///< Peripheral number 3.
    Four,             ///< Peripheral number 4.
    Five,             ///< Peripheral number 5.
    Six,              ///< Peripheral number 6.
    Seven,            ///< Peripheral number 7.
    Eight,            ///< Peripheral number 8.
    Nine,             ///< Peripheral number 9.
    Ten,              ///< Peripheral number 10.
    Unknown           ///< Unknown and invalid peripheral number.
};
///@typedef Register
///Hardware register address
using Register = volatile uint32_t *;
///@typedef InterruptFlags
///Interrupt flags
using InterruptFlags = uint32_t;
///@typedef InterruptCallback
///Interrupt callback
using InterruptCallback = void (*)(InterruptFlags flags);

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
 * @namespace IpTypes
 * @brief Types for IP connections
*/
namespace IpTypes {

    /**
     * @enum Version
     * @brief The version to use for the IP connection
    */
    enum class Version : uint8_t {
        Unknown = 0, ///< Unknown
        IPv4,        ///< Internet Protocol Version 4
        IPv6,        ///< Internet Protocol Version 6
        IPv4v6,      ///< Internet Protocol Version 4 and 6
    };

    /**
     * @enum Protocol
     * @brief The protocol to use for the IP connection
    */
    enum class Protocol : uint8_t {
        Unknown = 0, ///< Unknown
        Tcp,         ///< Transmission Control Protocol
        Udp          ///< User Datagram Protocol
    };
}

/**
 * @brief Converts a DateTime structure to a Unix time.
 * @param[in] dt The DateTime structure to convert.
 * @return The Unix time equivalent of the DateTime.
 * @note Remember the datetime data should represent the time and date since January 1st, 1970. If you provide a datetime
 *       with the year 2025 you are asking for the date which is 2025 years since 1970.
 * @sa Adapted from Freescale Semiconductor fsl_snvs_hp.c
 */
static constexpr UnixTime ToUnixTime(const DateTime &dt) {
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
    uint16_t daysSince = dt._year * daysInAYear;
    //Since the epoch of the given year with leap days included.
    daysSince += ((1970 + dt._year) / 4) - (1970 / 4);
    //We are at January of this year, now go to our month.
    daysSince += daysFromJanuaryToNow(dt._month);
    //Now go to the day for the month.
    if (dt._day > 0) {
        daysSince += ((uint32_t )dt._day - 1);
    }
    //If this year is a leap year and it's less than or equal February
    if (0 != dt._year && 0 == (dt._year & 3) && dt._year <= 2) {
        daysSince--;
    }

    uint32_t secondsSince = (daysSince * secondsInADay) + (dt._hour * secondsInAnHour) + (dt._minute * secondsInAMinute) + dt._second;

    return secondsSince;
}

/**
 * @brief Convert a Unix time to a date time
 * @param[in] seconds The unix time
 * @returns the DateTime which is the time and date passed since January 1st, 1970.
 */
static constexpr DateTime ToDateTime(const UnixTime &seconds) {
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
    dateSince._hour = (uint8_t)(secondsLeftToAddToDatetime / secondsInAnHour);
    secondsLeftToAddToDatetime %= secondsInAnHour;
    dateSince._minute = (uint8_t)(secondsLeftToAddToDatetime / secondsInAMinute);
    dateSince._second = (uint8_t)(secondsLeftToAddToDatetime % secondsInAMinute);
    uint16_t yearsSinceStartingFrom1970 = 0;

    while (daysSince > daysInAYear) {
        if ((dateSince._year & 3U) == 0U) {
            daysSince--;
        }

        daysSince -= daysInAYear;
        yearsSinceStartingFrom1970++;
    }

    dateSince._year = yearsSinceStartingFrom1970;

    for (int m = 0; m <= 12 && daysSince > 0; m++) {
        uint8_t daysInMonth = daysPerMonth(dateSince._year, m);
        if (0 != daysInMonth && daysSince <= daysInMonth) {
            dateSince._month = m;
        }
        else {
            daysSince -= daysInMonth;
        }
    }

    dateSince._day = daysSince;

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