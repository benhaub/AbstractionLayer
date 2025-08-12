/***************************************************************************//**
* @author   Ben Haubrich
* @file     RtcAbstraction.hpp
* @details  Abstraction for RTC communication.
* @ingroup  Abstractions
*******************************************************************************/
#ifndef __RTC_ABSTRACTION_HPP__
#define __RTC_ABSTRACTION_HPP__

//AbstractionLayer
#include "IcCommunicationProtocol.hpp"
//C++
#include <cassert>

/**
 * @class RtcAbstraction
 * @brief Abstraction layer for RTC communication.
 */
class RtcAbstraction {

    public:
    /// @brief Destructor.
    virtual ~RtcAbstraction() = default;

    /// @brief The tag for logging
    static constexpr char TAG[] = "Rtc";

    /**
     * @brief Initialize the RTC
     * @pre Configure the RTC first.
     * @sa setHardwareConfig()
     * @sa setDriverConfig()
     * @returns ErrorType::Success if the RTC was initialized successfully.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType init() = 0;
    /**
     * @brief De-initialize the RTC
     * @returns ErrorType::Success if the RTC was de-initialized successfully.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType deinit() = 0;
    /**
     * @brief Write the date and time to the RTC
     * @param[in] dateTime The date and time to set.
     * @returns ErrorType::Success if the date and time was set successfully.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType writeDate(const DateTime& dateTime) = 0;
    /**
     * @brief Read the date and time from the RTC
     * @param[out] dateTime The date and time to read.
     * @returns ErrorType::Success if the date and time was read successfully.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType readDate(DateTime& dateTime) = 0;
    /**
     * @brief Set the alarm
     * @param[in] dateTime The date and time to set the alarm to.
     * @returns ErrorType::Success if the alarm was set successfully.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType writeAlarm(const DateTime& dateTime) = 0;
    /**
     * @brief Read the alarm
     * @param[out] dateTime The date and time to read the alarm from.
     * @returns ErrorType::Success if the alarm was read successfully.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType readAlarm(DateTime& dateTime) = 0;
    /**
     * @brief Set the hour mode
     * @param[in] twentyFourHourMode True to set the RTC to support reading and writing 24-hour clock times. False for 12-hour.
     * @returns ErrorType::Success if the mode could be set.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType setHourMode(bool twentyFourHourMode) = 0;

    /**
     * @brief ic
     * @returns The IC device used to communicate with the RTC as a const reference
     */
    const IcCommunicationProtocol &icConst() const { assert(nullptr != _ic); return *_ic; }
    /**
     * @brief ic
     * @returns The IC device used to communicate with the RTC as a mutable reference
     */
    IcCommunicationProtocol &ic() { assert(nullptr != _ic); return *_ic; }

    /// @brief Set the IC device that this client communicates on.
    /// @param ic The IC device to use.
    ErrorType setIcDevice(IcCommunicationProtocol &ic) { _ic = &ic; return ErrorType::Success; }

    protected:
    /// @brief The Integrated circuit device that is being used to communicate with the RTC.
    IcCommunicationProtocol *_ic = nullptr;
};

#endif //__RTC_ABSTRACTION_HPP__