#include "Nxppcf8506aModule.hpp"
//AbstractionLayer Utilities
#include "Log.hpp"

ErrorType Nxppcf8506a::init() {
    return ErrorType::Success;
}

ErrorType Nxppcf8506a::deinit() {
    return ErrorType::Success;
}

ErrorType Nxppcf8506a::writeDate(const DateTime& dateTime) {
    stopClock();

    //To write the date to the RTC, we use the fact that register writes cause
    //an auto icrement to the register address. So if you write to addres 01h, then
    //then next write will be to address 02h. In this way, we can write the entire date
    //by first writing to the seconds register, and then making subsequent writes that will
    //continue to fill each of the registers with the value given.
    StaticString::Container dateTimeArray = StaticString::Data<7>();
    dateTimeArray->resize(0);
    dateTimeArray->push_back(toBinaryCodedDecimal(dateTime._second));
    dateTimeArray->push_back(toBinaryCodedDecimal(dateTime._minute));
    dateTimeArray->push_back(toBinaryCodedDecimal(dateTime._hour));
    dateTimeArray->push_back(toBinaryCodedDecimal(dateTime._day));
    dateTimeArray->push_back(toBinaryCodedDecimal(dateTime._weekday));
    dateTimeArray->push_back(toBinaryCodedDecimal(dateTime._month));
    dateTimeArray->push_back(toBinaryCodedDecimal(dateTime._year));

    IcCommunicationProtocolTypes::AdditionalCommunicationParameters params = {
        _I2cAddress,
        static_cast<uint8_t>(RegisterMap::Seconds)
    };

    ErrorType error = ic().txBlocking(dateTimeArray, Milliseconds(1000), params);


    startClock();

    return error;
}

ErrorType Nxppcf8506a::readDate(DateTime& dateTime) {
    StaticString::Container dateTimeArray = StaticString::Data<7>();
    IcCommunicationProtocolTypes::AdditionalCommunicationParameters params = {
        _I2cAddress,
        static_cast<uint8_t>(RegisterMap::Seconds)
    };

    //Same as reading. The registers will auto increment with each read. So If we read 7 times starting at the seconds register,
    //we will get all the date data from second to year.
    ErrorType error = ic().rxBlocking(dateTimeArray, Milliseconds(1000), params);

    if (ErrorType::Success == error) {
        dateTime._second = fromBinaryCodedDecimal(dateTimeArray[0]);
        dateTime._minute = fromBinaryCodedDecimal(dateTimeArray[1]);
        dateTime._hour = fromBinaryCodedDecimal(dateTimeArray[2]);
        dateTime._day = fromBinaryCodedDecimal(dateTimeArray[3]);
        dateTime._weekday = fromBinaryCodedDecimal(dateTimeArray[4]);
        dateTime._month = fromBinaryCodedDecimal(dateTimeArray[5]);
        dateTime._year = fromBinaryCodedDecimal(dateTimeArray[6]);
    }

    return error;
}

ErrorType Nxppcf8506a::writeAlarm(const DateTime& dateTime) {
    return ErrorType::NotImplemented;
}

ErrorType Nxppcf8506a::readAlarm(DateTime& dateTime) {
    return ErrorType::NotImplemented;
}

ErrorType Nxppcf8506a::setHourMode(bool twentyFourHourMode) {
    ErrorType error = ErrorType::Failure;
    constexpr Milliseconds timeout = 1000;
    StaticString::Container controlRegisterData = StaticString::Data<1>();
    IcCommunicationProtocolTypes::AdditionalCommunicationParameters params = {
        _I2cAddress,
        static_cast<uint8_t>(RegisterMap::Control1)
    };

    //Read the contents of the register so that we don't modify anything we didn't intend to.
    error = ic().rxBlocking(controlRegisterData, timeout, params);
    if (ErrorType::Success != error) {
        return error;
    }

    twentyFourHourMode ? controlRegisterData[0] &= ~(1 << 1) : controlRegisterData[0] |= (1 << 1);

    error = ic().txBlocking(controlRegisterData, timeout, params);

    return error;
}

ErrorType Nxppcf8506a::startClock() {
    ErrorType error = ErrorType::Failure;
    constexpr Milliseconds timeout = 1000;
    constexpr uint8_t stopBitPosition = 5;
    constexpr uint8_t externalTestBitPosition = 7;
    StaticString::Container controlRegisterData = StaticString::Data<1>();
    IcCommunicationProtocolTypes::AdditionalCommunicationParameters params = {
        _I2cAddress,
        static_cast<uint8_t>(RegisterMap::Control1)
    };

    //Read the contents of the register so that we don't modify anything we didn't intend to.
    error = ic().rxBlocking(controlRegisterData, timeout, params);
    if (ErrorType::Success != error) {
        return error;
    }

    controlRegisterData[0] &= ~(1 << stopBitPosition);
    controlRegisterData[0] &= ~(1 << externalTestBitPosition);

    error = ic().txBlocking(controlRegisterData, timeout, params);

    return error;
}

ErrorType Nxppcf8506a::stopClock() {
    ErrorType error = ErrorType::Failure;
    constexpr Milliseconds timeout = 1000;
    constexpr uint8_t stopBitPosition = 5;
    StaticString::Container controlRegisterData = StaticString::Data<1>();
    IcCommunicationProtocolTypes::AdditionalCommunicationParameters params = {
        _I2cAddress,
        static_cast<uint8_t>(RegisterMap::Control1)
    };

    //Read the contents of the register so that we don't modify anything we didn't intend to.
    error = ic().rxBlocking(controlRegisterData, timeout, params);
    if (ErrorType::Success != error) {
        return error;
    }

    controlRegisterData[0] |= (1 << stopBitPosition);

    error = ic().txBlocking(controlRegisterData, timeout, params);

    return error;
}

ErrorType Nxppcf8506a::softwareReset() {
    ErrorType error = ErrorType::Failure;
    constexpr Milliseconds timeout = 1000;
    constexpr uint8_t resetCode = 0x58;
    StaticString::Container controlRegisterData = StaticString::Data<1>();
    IcCommunicationProtocolTypes::AdditionalCommunicationParameters params = {
        _I2cAddress,
        static_cast<uint8_t>(RegisterMap::Control1)
    };

    //Read the contents of the register so that we don't modify anything we didn't intend to.
    error = ic().rxBlocking(controlRegisterData, timeout, params);
    if (ErrorType::Success != error) {
        return error;
    }

    //Pg. 55, Sect. 7.2.1.3 Software reset, datasheet PCF85063A datasheet.
    controlRegisterData[0] = resetCode;

    error = ic().txBlocking(controlRegisterData, timeout, params);

    stopClock();
    startClock();

    return error;
}
