//AbstractionLayer
#include "RtcModule.hpp"
#include "OperatingSystemModule.hpp"
#include "SpiModule.hpp"
//TI
#include "ti/drivers/net/wifi/device.h"

Rtc::Rtc() : RtcAbstraction() {
    Spi spi;
    Id thread;
    ErrorType error = spi.init();
    if (ErrorType::Success != error) {
        return;
    }

    constexpr Bytes kilobyte = 1024;
    //SIMPLELINK_THREAD_NAME is defined at compile time in cc32xx.cmake. Both Wifi and storage must create this thread in order to access the
    //API for Wifi and storage and may do so independently of each other. The Operating System must be check for this presence of this thread
    //before attempting to start it. Checking the return value of sl_start() is not sufficient since the wifi radio can be turned on or off at
    //any time.
    constexpr std::array<char, OperatingSystemTypes::MaxThreadNameLength> simplelinkThreadName = {SIMPLELINK_THREAD_NAME};
    if (ErrorType::NoData == OperatingSystem::Instance().threadId(simplelinkThreadName, thread)) {
        
        error = OperatingSystem::Instance().createThread(OperatingSystemTypes::Priority::High,
                                                simplelinkThreadName,
                                                nullptr,
                                                2*kilobyte,
                                                sl_Task,
                                                thread);
        
        assert(ErrorType::Success == error);
    }
}
ErrorType Rtc::init() {
    signed short result = sl_Start(NULL, NULL, NULL);
    return fromPlatformError(result);
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