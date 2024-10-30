#ifndef __RTC_MODULE_HPP__
#define __RTC_MODULE_HPP__

#include "RtcAbstraction.hpp"

class Rtc : public RtcAbstraction {
    public:
    Rtc() = default;
    ~Rtc() = default;

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType writeDate(const DateTime& dateTime) override;
    ErrorType readDate(DateTime& dateTime) override;
    ErrorType writeAlarm(const DateTime& dateTime) override;
    ErrorType readAlarm(DateTime& dateTime) override;
};

#endif //__RTC_MODULE_HPP__