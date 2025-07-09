#ifndef __RTC_MODULE_HPP__
#define __RTC_MODULE_HPP__

//AbstractionLayer
#include "RtcAbstraction.hpp"
//TI
#include "ti/drivers/net/wifi/device.h"

class Rtc final : public RtcAbstraction {
    public:
    Rtc();

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType writeDate(const DateTime& dateTime) override;
    ErrorType readDate(DateTime& dateTime) const override;
    ErrorType writeAlarm(const DateTime& dateTime) override;
    ErrorType readAlarm(DateTime& dateTime) override;
    ErrorType setHourMode(bool twentyFourHourMode) override;

    private:
    SlDateTime_t fromDateTime(const DateTime& dateTime) const;
    DateTime toDateTime(const SlDateTime_t& slDateTime) const;
};

#endif //__RTC_MODULE_HPP__