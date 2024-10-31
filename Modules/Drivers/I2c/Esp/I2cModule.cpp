//AbstractionLayer
#include "I2cModule.hpp"
//AbstractionLayer Applications
#include "Log.hpp"

#define I2C_ESP_MODULE_DEBUG 0

ErrorType I2c::init() {
    //You have to configure before initializing
    assert(I2cConfig::Mode::Unknown != _mode);
    assert(I2cConfig::Speed::Unknown != _speed);
    assert(I2cConfig::PeripheralNumber::Unknown != _peripheral);

    i2c_config_t conf;
    ErrorType error = ErrorType::Failure;

    conf.mode = toEspMode(_mode, error);
    conf.sda_io_num = toEspPinNumber(_sda);
    conf.scl_io_num = toEspPinNumber(_scl);
    conf.sda_pullup_en = _sdaPullup;
    conf.scl_pullup_en = _sclPullup;

    if (I2cConfig::Mode::Controller == _mode) {
        conf.master.clk_speed = toEspClockSpeed(_speed, error);
        conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
        if (ErrorType::Success != error) {
            return error;
        }
    }
    else if (I2cConfig::Mode::Target == _mode) {
        //conf.slave.addr_10bit_en
        //conf.slave.slave_addr
        //conf.slave.maximum_speed

        return ErrorType::NotImplemented;
    }

    const i2c_port_t i2cPort = toEspPort(_peripheral, error);
    i2c_param_config(i2cPort, &conf);

    if (I2cConfig::Mode::Controller == _mode) {
        error = toPlatformError(i2c_driver_install(i2cPort, conf.mode, 0, 0, 0));
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

    return toPlatformError(i2c_driver_delete(i2cPort));
}

ErrorType I2c::setHardwareConfig(const I2cConfig::PeripheralNumber peripheral, const I2cConfig::Mode mode, const I2cConfig::Speed speed, const PinNumber sda, const bool sdaPullup, const PinNumber scl, const bool sclPullup) {
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

ErrorType I2c::txBlocking(const std::string &data, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;
    i2c_port_t i2cPort = toEspPort(_peripheral, error);
    if (ErrorType::Success != error) {
        return error;
    }

    //Appen the register address to the beginning of the data.
    //TODO: 10-bit addressing?
#if I2C_ESP_MODULE_DEBUG
    CBT_LOGI(TAG, "Tx");
    CBT_LOG_BUFFER_HEXDUMP(TAG, data.data(), data.size(), LogType::Info);
#endif
    std::string writeData(data.size() + 1, 0);
    writeData.resize(0);
    writeData.push_back(static_cast<char>((uint8_t)_registerAddress));
    writeData.append(data);

#if I2C_ESP_MODULE_DEBUG
    CBT_LOGI(TAG, "Tx writeData");
    CBT_LOG_BUFFER_HEXDUMP(TAG, writeData.data(), writeData.size(), LogType::Info);
#endif

    if (I2cConfig::Mode::Controller == _mode) {
        //TODO: Need millisecondsToTicks
        error = toPlatformError(i2c_master_write_to_device(i2cPort, _deviceAddress, reinterpret_cast<const uint8_t *>(writeData.data()), writeData.size(), timeout));
    }
    else if (I2cConfig::Mode::Target == _mode) {
        error = ErrorType::NotImplemented;
    }
    else {
        error = ErrorType::NotSupported;
    }

    return error;
}
//TODO
ErrorType I2c::txNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType I2c::rxBlocking(std::string &buffer, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;
    uint8_t registerAddress = static_cast<uint8_t>(_registerAddress);
    i2c_port_t i2cPort = toEspPort(_peripheral, error);
    if (ErrorType::Failure == error) {
        return error;
    }

    if (I2cConfig::Mode::Controller == _mode) {
        error = toPlatformError(i2c_master_write_read_device(i2cPort, _deviceAddress, &registerAddress, 1, reinterpret_cast<uint8_t *>(buffer.data()), buffer.size(), timeout));
    }
    else if (I2cConfig::Mode::Target == _mode) {
        error = ErrorType::NotImplemented;
    }
    else {
        error = ErrorType::NotSupported;
    }

#if I2C_ESP_MODULE_DEBUG
    CBT_LOGI(TAG, "Rx");
    CBT_LOG_BUFFER_HEXDUMP(TAG, buffer.data(), buffer.size(), LogType::Info);
#endif

    return error;
}

ErrorType I2c::rxNonBlocking(std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType I2c::flushRxBuffer() {
    return ErrorType::NotSupported;
}