#ifndef __RTC_MODULE_HPP__
#define __RTC_MODULE_HPP__

#include "RtcAbstraction.hpp"
#include "I2cModule.hpp"

class Nxppcf8506a final : public RtcAbstraction {

    public:
    Nxppcf8506a() : RtcAbstraction() {}

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType writeDate(const DateTime& dateTime) override;
    ErrorType readDate(DateTime& dateTime) override;
    ErrorType writeAlarm(const DateTime& dateTime) override;
    ErrorType readAlarm(DateTime& dateTime) override;
    ErrorType setHourMode(bool twentyFourHourMode) override;
    ErrorType startClock();
    ErrorType stopClock();
    ErrorType softwareReset();

    private:
    static constexpr uint8_t _I2cAddress = 0b1010001;

    enum class RegisterMap : uint8_t {
        Control1 = 0,
        Control2,
        Offset,
        RamByte,
        Seconds,
        Minutes,
        Hours,
        Days,
        Weekdays,
        Months,
        Years,
        SecondAlarm,
        MinuteAlarm,
        HourAlarm,
        DayAlarm,
        WeekdayAlarm,
        TimerValue,
        TimerMode
    };

    uint8_t toBinaryCodedDecimal(uint8_t value) {
        uint8_t bcd = 0;
        bcd = value / 10 << 4;
        bcd |= value % 10;

        return bcd;
    }

    uint8_t fromBinaryCodedDecimal(uint8_t binaryCodedDecimal) {
        uint8_t value;

        value = binaryCodedDecimal & 0x0F;
        value += (binaryCodedDecimal >> 4) * 10;

        return value;
    }
};

#endif //__RTC_MODULE_HPP__