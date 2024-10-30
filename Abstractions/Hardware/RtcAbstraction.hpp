/***************************************************************************//**
* @author   Ben Haubrich
* @file     RtcAbstraction.hpp
* @details  Abstraction layer for RTC communication.
* @ingroup  AbstractionLayer
*******************************************************************************/
#ifndef __RTC_ABSTRACTION_HPP__
#define __RTC_ABSTRACTION_HPP__

//AbstractionLayer
#include <IcCommunicationProtocol.hpp>

/**
 * @class RtcAbstraction
 * @brief Abstraction layer for RTC communication.
 * @note All member functions can return any of ErrorType::NotImplemented, ErrorType::NotSupported or ErrorType::NotAvailable.
 */
class RtcAbstraction {
    public:
    /// @brief Constructor.
    RtcAbstraction() = default;
    /// @brief Destructor.
    virtual ~RtcAbstraction() = default;

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
     * @brief ic
     * @returns The IC device used to communicate with the RTC as a const reference
     */
    const std::unique_ptr<IcCommunicationProtocol> &icConst() const { return _ic; }
    /**
     * @brief ic
     * @returns The IC device used to communicate with the RTC as a mutable reference
     */
    std::unique_ptr<IcCommunicationProtocol> &ic() { return _ic; }

    protected:
    /// @brief The IC device that is being used to communicate with the RTC.
    std::unique_ptr<IcCommunicationProtocol> _ic;
};

#endif //__RTC_ABSTRACTION_HPP__