#include "RtcModule.hpp"
//AbstractionLayer Utilities
#include "Log.hpp"

ErrorType Rtc::init() {
    //Init and configure an IC device first.
    assert(nullptr != _ic);

    _i2c = dynamic_cast<I2c *>(_ic);
    if (nullptr == _i2c) {
        PLT_LOGE(TAG, "The NXP PCF85063a requires an I2C for communication");
        return ErrorType::PrerequisitesNotMet;
    }

    return ErrorType::Success;
}

ErrorType Rtc::deinit() {
    if (nullptr == _i2c) {
        return ErrorType::Success;
    }

    return ErrorType::Success;
}

ErrorType Rtc::writeDate(const DateTime& dateTime) {
    assert(isInitialized());

    stopClock();

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

    ErrorType error = _i2c->txBlocking(dateTimeArray, _I2cAddress, static_cast<uint8_t>(RegisterMap::Seconds), Milliseconds(1000));

    startClock();

    return error;
}

ErrorType Rtc::readDate(DateTime& dateTime) {
    assert(isInitialized());

    std::string dateTimeArray(7,0);

    //Same as reading. The registers will auto increment with each read. So If we read 7 times starting at the seconds register,
    //we will get all the date data from second to year.
    ErrorType error = _i2c->rxBlocking(dateTimeArray, _I2cAddress, static_cast<uint8_t>(RegisterMap::Seconds), Milliseconds(1000));

    dateTime.second = fromBinaryCodedDecimal(dateTimeArray.at(0));
    dateTime.minute = fromBinaryCodedDecimal(dateTimeArray.at(1));
    dateTime.hour = fromBinaryCodedDecimal(dateTimeArray.at(2));
    dateTime.day = fromBinaryCodedDecimal(dateTimeArray.at(3));
    dateTime.weekday = fromBinaryCodedDecimal(dateTimeArray.at(4));
    dateTime.month = fromBinaryCodedDecimal(dateTimeArray.at(5));
    dateTime.year = fromBinaryCodedDecimal(dateTimeArray.at(6));

    return ErrorType::Success;
}

//TODO:
ErrorType Rtc::writeAlarm(const DateTime& dateTime) {
    assert(isInitialized());

    return ErrorType::NotImplemented;
}

ErrorType Rtc::readAlarm(DateTime& dateTime) {
    assert(isInitialized());

    return ErrorType::NotImplemented;
}

ErrorType Rtc::setHourMode(bool twentyFourHourMode) {
    assert(isInitialized());

    ErrorType error = ErrorType::Failure;
    constexpr Milliseconds timeout = 1000;
    std::string controlRegisterData(1, 0);

    //Read the contents of the register so that we don't modify anything we didn't intend to.
    error = _i2c->rxBlocking(controlRegisterData, _I2cAddress, static_cast<uint8_t>(RegisterMap::Control1), timeout);
    if (ErrorType::Success != error) {
        return error;
    }

    twentyFourHourMode ? controlRegisterData.at(0) &= ~(1 << 1) : controlRegisterData.at(0) |= (1 << 1);

    error = _i2c->txBlocking(controlRegisterData, _I2cAddress, static_cast<uint8_t>(RegisterMap::Control1), timeout);

    return error;
}

ErrorType Rtc::startClock() {
    assert(isInitialized());

    ErrorType error = ErrorType::Failure;
    constexpr Milliseconds timeout = 1000;
    constexpr uint8_t stopBitPosition = 5;
    constexpr uint8_t externalTestBitPosition = 7;
    std::string controlRegisterData(1, 0);

    //Read the contents of the register so that we don't modify anything we didn't intend to.
    error = _i2c->rxBlocking(controlRegisterData, _I2cAddress, static_cast<uint8_t>(RegisterMap::Control1), timeout);
    if (ErrorType::Success != error) {
        return error;
    }

    controlRegisterData.at(0) &= ~(1 << stopBitPosition);
    controlRegisterData.at(0) &= ~(1 << externalTestBitPosition);

    error = _i2c->txBlocking(controlRegisterData, _I2cAddress, static_cast<uint8_t>(RegisterMap::Control1), timeout);

    return error;
}

ErrorType Rtc::stopClock() {
    assert(isInitialized());

    ErrorType error = ErrorType::Failure;
    constexpr Milliseconds timeout = 1000;
    constexpr uint8_t stopBitPosition = 5;
    std::string controlRegisterData(1, 0);

    //Read the contents of the register so that we don't modify anything we didn't intend to.
    error = _i2c->rxBlocking(controlRegisterData, _I2cAddress, static_cast<uint8_t>(RegisterMap::Control1), timeout);
    if (ErrorType::Success != error) {
        return error;
    }

    controlRegisterData.at(0) |= (1 << stopBitPosition);

    error = _i2c->txBlocking(controlRegisterData, _I2cAddress, static_cast<uint8_t>(RegisterMap::Control1), timeout);

    return error;
}

ErrorType Rtc::softwareReset() {
    assert(isInitialized());

    ErrorType error = ErrorType::Failure;
    constexpr Milliseconds timeout = 1000;
    constexpr uint8_t resetCode = 0x58;
    std::string controlRegisterData(1, 0);

    //Read the contents of the register so that we don't modify anything we didn't intend to.
    error = _i2c->rxBlocking(controlRegisterData, _I2cAddress, static_cast<uint8_t>(RegisterMap::Control1), timeout);
    if (ErrorType::Success != error) {
        return error;
    }

    //Pg. 55, Sect. 7.2.1.3 Software reset, datasheet PCF85063A datasheet.
    controlRegisterData.at(0) = resetCode;

    error = _i2c->txBlocking(controlRegisterData, _I2cAddress, static_cast<uint8_t>(RegisterMap::Control1), timeout);

    stopClock();
    startClock();

    return error;
}
