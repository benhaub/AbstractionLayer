#ifndef __RTC_MODULE_HPP__
#define __RTC_MODULE_HPP__

#include "RtcAbstraction.hpp"

class Rtc final : public RtcAbstraction {
    public:
    Rtc() : RtcAbstraction() {}

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType writeDate(const DateTime& dateTime) override;
    ErrorType readDate(DateTime& dateTime) override;
    ErrorType writeAlarm(const DateTime& dateTime) override;
    ErrorType readAlarm(DateTime& dateTime) override;
    ErrorType setHourMode(bool twentyFourHourMode) override;
};

#endif //__RTC_MODULE_HPP__