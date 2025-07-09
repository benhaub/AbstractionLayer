//AbstractionLayer
#include "I2cModule.hpp"
#include "Log.hpp"
#include "OperatingSystemModule.hpp"

#define I2C_ESP_MODULE_DEBUG 1

ErrorType I2c::init() {
    //You have to configure before initializing
    if ((I2cTypes::Mode::Unknown == _mode) || (I2cTypes::Speed::Unknown == _speed) || (PeripheralNumber::Unknown == _peripheral)) {
        return ErrorType::PrerequisitesNotMet;
    }

    i2c_config_t conf;
    ErrorType error = ErrorType::Failure;

    conf.mode = toEspMode(_mode, error);
    conf.sda_io_num = toEspPinNumber(_sda);
    conf.scl_io_num = toEspPinNumber(_scl);
    conf.sda_pullup_en = _sdaPullup;
    conf.scl_pullup_en = _sclPullup;

    if (I2cTypes::Mode::Controller == _mode) {
        conf.master.clk_speed = toEspClockSpeed(_speed, error);
        conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
        if (ErrorType::Success != error) {
            return error;
        }
    }
    else if (I2cTypes::Mode::Target == _mode) {
        //conf.slave.addr_10bit_en
        //conf.slave.slave_addr
        //conf.slave.maximum_speed

        return ErrorType::NotImplemented;
    }

    const i2c_port_t i2cPort = toEspPort(_peripheral, error);
    i2c_param_config(i2cPort, &conf);

    if (I2cTypes::Mode::Controller == _mode) {
        error = fromPlatformError(i2c_driver_install(i2cPort, conf.mode, 0, 0, 0));
    }
    else {
        error = ErrorType::NotImplemented;
    }

    return error;
}

ErrorType I2c::deinit() {
    ErrorType error;
    const i2c_port_t i2cPort = toEspPort(_peripheral, error);
    if (ErrorType::Success != error) {
        return error;
    }

    return fromPlatformError(i2c_driver_delete(i2cPort));
}

ErrorType I2c::setHardwareConfig(const PeripheralNumber peripheral, const I2cTypes::Mode mode, const I2cTypes::Speed speed, const PinNumber sda, const bool sdaPullup, const PinNumber scl, const bool sclPullup) {
    _mode = mode;
    _speed = speed;
    _peripheral = peripheral;
    _sda = sda;
    _sdaPullup = sdaPullup;
    _scl = scl;
    _sclPullup = sclPullup;

    return ErrorType::Success;
}

ErrorType I2c::setInterruptConfig(const bool arbitrationLost, const bool nackDetected, const bool sclLowTimeout, const bool stopDetect, const bool receiveFifoOverflow, const bool transmitFifoOverflow) {
    //See the API reference docs. An interrupt handler is created by default when you call i2c_driver_install.
    return ErrorType::NotSupported;
}

ErrorType I2c::txBlocking(const std::string &data, uint8_t deviceAddress, uint8_t registerAddress, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;
    i2c_port_t i2cPort = toEspPort(_peripheral, error);
    if (ErrorType::Success != error) {
        return error;
    }

    //Append the register address to the beginning of the data.
    //TODO: 10-bit addressing?
    std::string writeData(data.size() + 1, 0);
    writeData.resize(0);
    writeData.push_back(static_cast<char>((uint8_t)registerAddress));
    writeData.append(data);

#if I2C_ESP_MODULE_DEBUG
    PLT_LOGI(TAG, "Tx");
    PLT_LOG_BUFFER_HEXDUMP(TAG, writeData.data(), writeData.size(), LogType::Info);
#endif

    if (I2cTypes::Mode::Controller == _mode) {
        Ticks timeToWait;
        OperatingSystem::Instance().millisecondsToTicks(timeout, timeToWait);
        error = fromPlatformError(i2c_master_write_to_device(i2cPort, deviceAddress, reinterpret_cast<const uint8_t *>(writeData.data()), writeData.size(), timeToWait));
    }
    else if (I2cTypes::Mode::Target == _mode) {
        error = ErrorType::NotImplemented;
    }
    else {
        error = ErrorType::NotSupported;
    }

    return error;
}

ErrorType I2c::txNonBlocking(const std::shared_ptr<std::string> data, uint8_t deviceAddress, uint8_t registerAddress, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType I2c::rxBlocking(std::string &buffer, uint8_t deviceAddress, uint8_t registerAddress, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;
    i2c_port_t i2cPort = toEspPort(_peripheral, error);
    if (ErrorType::Failure == error) {
        return error;
    }

    if (I2cTypes::Mode::Controller == _mode) {
        error = fromPlatformError(i2c_master_write_read_device(i2cPort, deviceAddress, &registerAddress, 1, reinterpret_cast<uint8_t *>(buffer.data()), buffer.size(), timeout));
    }
    else if (I2cTypes::Mode::Target == _mode) {
        error = ErrorType::NotImplemented;
    }
    else {
        error = ErrorType::NotSupported;
    }

#if I2C_ESP_MODULE_DEBUG
    PLT_LOGI(TAG, "Rx");
    PLT_LOG_BUFFER_HEXDUMP(TAG, buffer.data(), buffer.size(), LogType::Info);
#endif

    return error;
}

ErrorType I2c::rxNonBlocking(std::shared_ptr<std::string> buffer, uint8_t deviceAddress, uint8_t registerAddress, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType I2c::flushRxBuffer() {
    return ErrorType::NotSupported;
}