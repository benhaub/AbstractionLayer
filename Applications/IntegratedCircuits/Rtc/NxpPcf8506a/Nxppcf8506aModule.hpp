#ifndef __NXPPCF8506A_MODULE_HPP__
#define __NXPPCF8506A_MODULE_HPP__

#include "RtcAbstraction.hpp"
#include "I2cModule.hpp"

/**
 * @class Nxppcf8506a
 * @brief NXP PCF8506A Real Time Clock
 */
class Nxppcf8506a final : public RtcAbstraction {

    public:
    /// @brief Constructor
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
    /// @brief The I2C address of the PCF8506A
    static constexpr uint8_t _I2cAddress = 0b1010001;

    /**
     * @enum RegisterMap
     * @brief The register map of the PCF8506A
     */
    enum class RegisterMap : uint8_t {
        Control1 = 0, ///< Control register 1
        Control2,     ///< Control register 2
        Offset,       ///< Offset register
        RamByte,      ///< Ram byte register
        Seconds,      ///< Seconds register
        Minutes,      ///< Minutes register
        Hours,        ///< Hours register
        Days,         ///< Days register
        Weekdays,     ///< Weekdays register
        Months,       ///< Months register
        Years,        ///< Years register
        SecondAlarm,  ///< Second alarm register
        MinuteAlarm,  ///< Minute alarm register
        HourAlarm,    ///< Hour alarm register
        DayAlarm,     ///< Day alarm register
        WeekdayAlarm, ///< Weekday alarm register
        TimerValue,   ///< Timer value register
        TimerMode     ///< Timer mode register
    };

    /**
     * @brief Convert a value to binary coded decimal
     * @param value The value to convert
     * @returns The value in binary coded decimal
     */
    uint8_t toBinaryCodedDecimal(uint8_t value) {
        uint8_t bcd = 0;
        bcd = value / 10 << 4;
        bcd |= value % 10;

        return bcd;
    }

    /**
     * @brief Convert a value from binary coded decimal
     * @param binaryCodedDecimal The value in binary coded decimal
     * @returns The value converted from binary coded decimal
     */
    uint8_t fromBinaryCodedDecimal(uint8_t binaryCodedDecimal) {
        uint8_t value;

        value = binaryCodedDecimal & 0x0F;
        value += (binaryCodedDecimal >> 4) * 10;

        return value;
    }
};

#endif //__NXPPCF8506A_MODULE_HPP__