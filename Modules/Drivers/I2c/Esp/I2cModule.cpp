//AbstractionLayer
#include "I2cModule.hpp"

ErrorType I2cModule::init() {
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

    if (I2cConfig::Mode::Master == _mode) {
        conf.master.clk_speed = toEspClockSpeed(_speed, error);
    }
    else if (I2cConfig::Mode::Slave == _mode) {
        //conf.slave.addr_10bit_en
        //conf.slave.slave_addr
        //conf.slave.maximum_speed

        return ErrorType::NotImplemented;
    }

    const i2c_port_t i2cPort = toEspPort(_peripheral, error);
    i2c_param_config(i2cPort, &conf);

    if (I2cConfig::Mode::Master == _mode) {
        error = toPlatformError(i2c_driver_install(i2cPort, conf.mode, 0, 0, 0));
    }
    else {
        error = ErrorType::NotImplemented;
    }

    return error;
}

ErrorType I2cModule::deinit() {
    ErrorType error;
    const i2c_port_t i2cPort = toEspPort(_peripheral, error);
    if (ErrorType::Success != error) {
        return error;
    }

    return toPlatformError(i2c_driver_delete(i2cPort));
}

ErrorType I2cModule::setHardwareConfig(const I2cConfig::PeripheralNumber peripheral, const I2cConfig::Mode mode, const I2cConfig::Speed speed, const PinNumber sda, const bool sdaPullup, const PinNumber scl, const bool sclPullup) {
    _mode = mode;
    _speed = speed;
    _peripheral = peripheral;
    _sda = sda;
    _sdaPullup = sdaPullup;
    _scl = scl;
    _sclPullup = sclPullup;

    return ErrorType::Success;
}

ErrorType I2cModule::txBlocking(const std::string &data, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;
    i2c_port_t i2cPort = toEspPort(_peripheral, error);
    if (ErrorType::Success != error) {
        return error;
    }

    if (I2cConfig::Mode::Master == _mode) {
        //TODO: Need millisecondsToTicks
        error = toPlatformError(i2c_master_write_to_device(i2cPort, _currentWriteAddress, reinterpret_cast<const uint8_t *>(data.data()), data.size(), timeout));
    }
    else if (I2cConfig::Mode::Slave == _mode) {
        error = ErrorType::NotImplemented;
    }
    else {
        error = ErrorType::NotSupported;
    }

    return error;
}

ErrorType I2cModule::txNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType I2cModule::rxBlocking(std::string &buffer, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;
    uint8_t registerAddress;
    i2c_port_t i2cPort = toEspPort(_peripheral, error);
    if (ErrorType::Failure == error) {
        return error;
    }

    if (I2cConfig::Mode::Master == _mode) {
        error = toPlatformError(i2c_master_write_read_device(i2cPort, _currentReadAddress, &registerAddress, sizeof(registerAddress), reinterpret_cast<uint8_t *>(buffer.data()), buffer.size(), timeout));
    }
    else if (I2cConfig::Mode::Slave == _mode) {
        error = ErrorType::NotImplemented;
    }
    else {
        error = ErrorType::NotSupported;
    }

    return error;
}

ErrorType I2cModule::rxNonBlocking(std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType I2cModule::flushRxBuffer() {
    return ErrorType::NotSupported;
}