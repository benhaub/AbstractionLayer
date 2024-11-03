/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   Types.hpp
* @details \b Synopsis: \n Variable types for enhanced readability.
* @ingroup AbstractionLayer
*******************************************************************************/

#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <cstdint>
#include <cassert>

//-------------------------------Time
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
///Date and time
struct DateTime {
    uint8_t second; ///< Seconds. 0-59
    uint8_t minute; ///< minutes. 0-59
    uint8_t hour;   ///< hour. 0-23
    uint8_t day;    ///< day. 0-31
    uint8_t weekday;///< week 0-7 (Sun-Sat)
    uint8_t month;  ///< month. 0-12
    uint8_t year;   ///< year. 0-99.
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

//-------------------------------Networks
///@typedef Port
///Network port number
using Port = uint16_t;
///@typedef Socket
///Network socket number
using Socket = int32_t;

//Adapted from Freescale Semiconductor fsl_snvs_hp.c
static constexpr UnixTime ToUnixTime(DateTime dt) {
    constexpr uint8_t yearPrefix = 20;
    constexpr uint16_t daysInAYear = 365;
    constexpr uint32_t secondsInADay = 86400;
    constexpr uint16_t secondsInAnHour = 3600;
    constexpr uint16_t secondsInAMinute = 60;
    //Number of days from the beginning of the non leap-year
    auto daysFromJanuaryToNow = [](uint8_t month) {
        if (month <= 12 && month >= 1) {
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
    uint16_t daysSince = (((20 + (uint32_t)dt.year) - 1970) * daysInAYear);
    //Since the epoch of the given year with leap days included.
    daysSince += (((uint32_t)dt.year / 4) - (1970 / 4));
    //We are at January of this year, now go to our month.
    daysSince += daysFromJanuaryToNow(dt.month);
    //Now go to the day for the month.
    daysSince += ((uint32_t )dt.day - 1);
    //If this year is a leap year and it's less than or equal February
    if (0 == (dt.year & 3) && dt.year <= 2) {
        daysSince--;
    }

    uint32_t secondsSince = (daysSince * secondsInADay) + (dt.hour * secondsInAnHour) + (dt.minute * secondsInAMinute) + dt.second;

    return secondsSince;
}

static constexpr DateTime ToDateTime(UnixTime seconds) {
    constexpr uint8_t yearPrefix = 20;
    constexpr uint16_t daysInAYear = 365;
    constexpr uint32_t secondsInADay = 86400;
    constexpr uint16_t secondsInAnHour = 3600;
    constexpr uint16_t secondsInAMinute = 60;

    auto daysPerMonth = [](uint16_t currentYear, uint8_t month) {
        if (month <= 12 && month >= 1) {
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
    uint16_t daysSince = seconds / secondsInADay + 1;
    secondsLeftToAddToDatetime %= secondsInADay;

    DateTime dateSince {};
    dateSince.hour = (uint8_t)(secondsLeftToAddToDatetime / secondsInAnHour);
    secondsLeftToAddToDatetime %= secondsInAnHour;
    dateSince.minute = (uint8_t)(secondsLeftToAddToDatetime / 60);
    dateSince.second = (uint8_t)(secondsLeftToAddToDatetime / secondsInAMinute);
    uint16_t yearsSinceStartingFrom1970 = 1970;
    uint16_t numberOfLeapYearsSince1970 = 0;

    while (daysSince > daysInAYear + numberOfLeapYearsSince1970) {
       daysSince -= daysInAYear;
       yearsSinceStartingFrom1970++;

       if (0 == (yearsSinceStartingFrom1970 & 3)) {
          numberOfLeapYearsSince1970++; 
       }
    }

    dateSince.year = yearsSinceStartingFrom1970;

    for (int m = 0; m <= 12; m++) {
        if (daysSince <= daysPerMonth(dateSince.year, m)) {
            dateSince.month = m;
        }
        else {
            daysSince -= daysPerMonth(dateSince.year, m);
        }
    }

    dateSince.day = daysSince;

    return dateSince;
} 

#endif //__TYPES_HPP__