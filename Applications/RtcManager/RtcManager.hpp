/***************************************************************************//**
* @author   Ben Haubrich
* @file     RtcManager.hpp
* @details  Manages updating, setting, and synchronizing clocks used by the Application
* @ingroup  Applications
*******************************************************************************/
#ifndef __RTC_MANAGER_HPP__
#define __RTC_MANAGER_HPP__

//AbstractionLayer
#include "RtcModule.hpp"

/**
 * @namespace RtcManagerTypes
 * @brief Contains types used by the RtcManager
 */
namespace RtcManagerTypes {

    /**
     * @struct Criteria
     * @brief The conditions that must be met while setting and syncing the RTCs
     */
    struct Criteria {
        Seconds maxDifferenceBetweenInternalAndExternalRtc;  ///< The maximum allowed difference between the internal and external RTCs
        Seconds maxDifferenceBetweenInternalRtcAndInputTime; ///< The maximum allowed difference between the internal RTC and the input time
    };
}

/**
 * @class RtcManager
 * @brief Manages updating, setting, and synchronizing clocks
 *        used by the Application
 * @details Given the clocks set by the user, this class will allow the user to set
 *          the internal RTC with the time given if it meets the criteria set by thee
 *          user. It will then automatically update the external RTC if any drift has
 *          occurred beyond the threshold set by the user.
 */
class RtcManager final {

    public:
    /**
     * @brief Constrcutor
     * @details A read of the external RTC will be done to initailize member variables.
     * @param[in] criteria The criteria to use for setting and synchronizing the RTCs
     * @param[in] manageExternalRtc If true, the external RTC will be managed according to the specified criteria
     * @param[in] manageInternalRtc If true, the internal RTC will be managed according to the specified criteria
     */
    RtcManager(const RtcManagerTypes::Criteria &criteria, const bool manageExternalRtc, const bool manageInternalRtc) : _criteria(criteria) {
        if (manageExternalRtc) {
            _externalRtc = std::make_unique<Rtc>();
            assert(nullptr != _externalRtc);
            _externalRtc->readDate(_lastExternalRtcTimeQueried);
        }
        if (manageInternalRtc) {
            _internalRtc = std::make_unique<Rtc>();
            assert(nullptr != _internalRtc);
        }
    }

    ///@brief Tag used for logging
    static constexpr char TAG[] = "RtcManager";

    /**
     * @brief Set the criteria for setting and synchronizing the RTCs
     * @param[in] criteria The criteria to set
     * @sa RtcManagerTypes::Criteria
     * @returns ErrorType::Success if the criteria was set
     * @returns ErrorType::Failure otherwise.
     */
    ErrorType setCriteria(const RtcManagerTypes::Criteria &criteria) { _criteria = criteria; return ErrorType::Success; }
    /**
     * @brief Submit a new input time for the managers consideration
     * @param[in] inputTime The time to submit
     * @returns ErrorType::Success if the time was set
     * @returns ErrorType::PrerequistitesNotMet if the input time did not meet the criteria.
     * @returns ErrorType::Failure otherwise.
     */
    ErrorType submitInputTime(const UnixTime &inputTime);
    /**
     * @brief Get the time from the internal RTC.
     * @param[out] dateTime The time from the internal RTC.
     * @returns ErrorType::Success if the time was retrieved
     * @returns ErrorType::Failure otherwise.
     */
    ErrorType internalRtcTime(DateTime &dateTime) const;
    /**
     * @brief Get the time from the external RTC.
     * @param[in] forceQuery If true, this function will query the external RTC for it's current time and return it.
     *                   Otherwise, the time will be the last time that was queried.
     * @param[out] dateTime The time from the external RTC.
     * @details It's much faster to return the last time that was queried rather than force another query since
     *          a new query will block while the external RTC is communicated with.
     * @returns ErrorType::Success if the time was retrieved
     * @returns ErrorType::Failure otherwise.
     */
    ErrorType externalRtcTime(const bool forceQuery, DateTime &dateTime);

    /// @brief Get the internal RTC as a constant reference
    const RtcAbstraction &internalRtc() const { assert(nullptr != _internalRtc.get()); return *_internalRtc; }
    /// @brief Get the external RTC as a constant reference
    const RtcAbstraction &externalRtc() const { assert(nullptr != _externalRtc.get()); return *_externalRtc; }

    private:
    /// @brief The internal RTC
    std::unique_ptr<RtcAbstraction> _internalRtc;
    /// @brief The external RTC
    std::unique_ptr<RtcAbstraction> _externalRtc;
    /// @brief The criteria to use for setting and synchronizing the RTCs
    RtcManagerTypes::Criteria _criteria = {0, 0};
    /// @brief The last time that was queried from the external RTC
    DateTime _lastExternalRtcTimeQueried = DateTime();
};

#endif //__RTC_MANAGER_HPP__