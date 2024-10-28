/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   Types.hpp
* @details \b Synopsis: \n Variable types for enhanced readability.
* @ingroup AbstractionLayer
*******************************************************************************/

#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <cstdint>

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
    Seconds seconds;
    Minutes minutes;
    Hours hours;
    uint8_t week;
    uint8_t month;
    uint16_t year;
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

#endif //__TYPES_HPP__