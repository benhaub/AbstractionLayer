#include "RtcModule.hpp"
//AbstractionLayer Utilities
#include "Log.hpp"

ErrorType Rtc::init() {
    //Init and configure an IC device first.
    assert(nullptr != _ic.get());

    _i2c = dynamic_cast<I2c *>(_ic.get());
    if (nullptr == _i2c) {
        CBT_LOGE(TAG, "The NXP PCF85063a requires an I2C for communication");
        return ErrorType::PrerequisitesNotMet;
    }

    _i2c->deviceAddress() = static_cast<uint8_t>(_SlaveAddress);

    return ErrorType::Success;
}

ErrorType Rtc::deinit() {
    if (nullptr == _i2c) {
        return ErrorType::Success;
    }

    return ErrorType::Success;
}
//TODO: 24 hour mode?
ErrorType Rtc::writeDate(const DateTime& dateTime) {
    assert(isInitialized());

    //To write the date to the RTC, we use the fact that register writes cause
    //an auto icrement to the register address. So if you write to addres 01h, then
    //then next write will be to address 02h. In this way, we can write the entire date
    //by first writing to the seconds register, and then making subsequent writes that will
    //continue to fill each of the registers with the value given.
    std::string dateTimeArray(7, 0);
    dateTimeArray.resize(0);
    dateTimeArray.push_back(toBinaryCodedDecimal(dateTime.second));
    dateTimeArray.push_back(toBinaryCodedDecimal(dateTime.minute));
    dateTimeArray.push_back(toBinaryCodedDecimal(dateTime.hour));
    dateTimeArray.push_back(toBinaryCodedDecimal(dateTime.day));
    dateTimeArray.push_back(toBinaryCodedDecimal(dateTime.weekday));
    dateTimeArray.push_back(toBinaryCodedDecimal(dateTime.month));
    dateTimeArray.push_back(toBinaryCodedDecimal(dateTime.year));

    _i2c->registerAddress() = static_cast<uint8_t>(RegisterMap::Seconds);
    ErrorType error = _ic->txBlocking(dateTimeArray, Milliseconds(1000));

    return error;
}

ErrorType Rtc::readDate(DateTime& dateTime) {
    assert(isInitialized());

    std::string dateTimeArray(7,0);

    //Same as reading. The registers will auto increment with each read. So If we read 7 times starting at the seconds register,
    //we will get all the date data from second to year.
    _i2c->registerAddress() = static_cast<uint8_t>(RegisterMap::Seconds);
    ErrorType error = _i2c->rxBlocking(dateTimeArray, Milliseconds(1000));

    dateTime.second = fromBinaryCodedDecimal(dateTimeArray.at(0));
    dateTime.minute = fromBinaryCodedDecimal(dateTimeArray.at(1));
    dateTime.hour = fromBinaryCodedDecimal(dateTimeArray.at(2));
    dateTime.day = fromBinaryCodedDecimal(dateTimeArray.at(3));
    dateTime.weekday = fromBinaryCodedDecimal(dateTimeArray.at(4));
    dateTime.month = fromBinaryCodedDecimal(dateTimeArray.at(5));
    dateTime.year = fromBinaryCodedDecimal(dateTimeArray.at(6));

    return ErrorType::Success;
}

ErrorType Rtc::writeAlarm(const DateTime& dateTime) {
    assert(isInitialized());

    return ErrorType::NotImplemented;
}

ErrorType Rtc::readAlarm(DateTime& dateTime) {
    assert(isInitialized());

    return ErrorType::NotImplemented;
}