//AbstractionLayer
#include "RtcModule.hpp"
#include "OperatingSystemModule.hpp"
#include "SpiModule.hpp"
//TI
#include "ti/drivers/net/wifi/device.h"

ErrorType Rtc::init() {
    Spi spi;
    ErrorType error = spi.init();

    if (ErrorType::Success == error) {
        error = OperatingSystem::Instance().startSimpleLinkTask();

        if (ErrorType::Success == error) {
            signed short result = sl_Start(NULL, NULL, NULL);
            error = fromPlatformError(result);
        }
    }

    return error;
}

ErrorType Rtc::deinit() {
    signed short result = sl_Stop(0xFFFF);
    return fromPlatformError(result);
}

ErrorType Rtc::writeDate(const DateTime& dateTime) {
    SlDateTime_t slDateTime = fromDateTime(dateTime);
    signed short result = sl_DeviceSet(SL_DEVICE_GENERAL, SL_DEVICE_GENERAL_DATE_TIME, sizeof(slDateTime), (uint8_t *)&slDateTime);
    return fromPlatformError(result);
}

ErrorType Rtc::readDate(DateTime& dateTime) {
    SlDateTime_t slDateTime;
    _u8 configOption;
    _u16 configLength;
    configOption = SL_DEVICE_GENERAL_DATE_TIME;

    signed short result = sl_DeviceGet(SL_DEVICE_GENERAL, &configOption, &configLength, (_u8 *)&slDateTime);

    if (0 == result) {
        dateTime = toDateTime(slDateTime);
    }

    return fromPlatformError(result);
}

ErrorType Rtc::writeAlarm(const DateTime& dateTime) {
    return ErrorType::NotAvailable;
}

ErrorType Rtc::readAlarm(DateTime& dateTime) {
    return ErrorType::NotAvailable;
}

ErrorType Rtc::setHourMode(bool twentyFourHourMode) {
    return ErrorType::NotAvailable;
}

SlDateTime_t Rtc::fromDateTime(const DateTime& dateTime) const {
    SlDateTime_t slDateTime;
    slDateTime.tm_year = dateTime._year + 1970;
    slDateTime.tm_mon  = dateTime._month;
    slDateTime.tm_day  = dateTime._day;
    slDateTime.tm_hour = dateTime._hour;
    slDateTime.tm_min  = dateTime._minute;
    slDateTime.tm_sec  = dateTime._second;
    return slDateTime;
}

DateTime Rtc::toDateTime(const SlDateTime_t& slDateTime) const {
    DateTime dateTime;
    dateTime._year = slDateTime.tm_year - 1970;
    dateTime._month = slDateTime.tm_mon;
    dateTime._day = slDateTime.tm_day;
    dateTime._hour = slDateTime.tm_hour;
    dateTime._minute = slDateTime.tm_min;
    dateTime._second = slDateTime.tm_sec;
    return dateTime;
}