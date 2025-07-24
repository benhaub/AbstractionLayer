//AbstractionLayer
#include "I2cModule.hpp"
#include "Log.hpp"
#include "OperatingSystemModule.hpp"

#define I2C_ESP_MODULE_DEBUG 1

ErrorType I2c::init() {
    //You have to configure before initializing
    const bool configurationWasSet = (I2cTypes::Mode::Unknown != i2cParams().hardwareConfig.mode) && (I2cTypes::Speed::Unknown != i2cParams().hardwareConfig.speed) && (PeripheralNumber::Unknown != i2cParams().hardwareConfig.peripheral);
    if (configurationWasSet) {
        i2c_config_t conf;
        ErrorType error = ErrorType::Failure;
        const I2cTypes::Mode mode = i2cParams().hardwareConfig.mode;
        const I2cTypes::Speed speed = i2cParams().hardwareConfig.speed;
        const PeripheralNumber peripheral = i2cParams().hardwareConfig.peripheral;
        const PinNumber sda = i2cParams().hardwareConfig.sda;
        const bool sdaPullup = i2cParams().hardwareConfig.sdaPullup;
        const PinNumber scl = i2cParams().hardwareConfig.scl;
        const bool sclPullup = i2cParams().hardwareConfig.sclPullup;

        conf.mode = toEspMode(mode, error);
        conf.sda_io_num = toEspPinNumber(sda);
        conf.scl_io_num = toEspPinNumber(scl);
        conf.sda_pullup_en = sdaPullup;
        conf.scl_pullup_en = sclPullup;

        if (I2cTypes::Mode::Controller == mode) {
            conf.master.clk_speed = toEspClockSpeed(speed, error);
            conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
            if (ErrorType::Success == error) {
                if (I2cTypes::Mode::Target == mode) {
                    //conf.slave.addr_10bit_en
                    //conf.slave.slave_addr
                    //conf.slave.maximum_speed

                    error = ErrorType::NotImplemented;
                }
                else {
                    const i2c_port_t i2cPort = toEspPort(peripheral, error);
                    i2c_param_config(i2cPort, &conf);

                    if (I2cTypes::Mode::Controller == mode) {
                        error = fromPlatformError(i2c_driver_install(i2cPort, conf.mode, 0, 0, 0));
                    }
                    else {
                        error = ErrorType::NotImplemented;
                    }
                }
            }
        }

        return error;
    }
}

ErrorType I2c::deinit() {
    ErrorType error;
    const PeripheralNumber peripheral = i2cParams().hardwareConfig.peripheral;

    const i2c_port_t i2cPort = toEspPort(peripheral, error);
    if (ErrorType::Success != error) {
        return error;
    }

    return fromPlatformError(i2c_driver_delete(i2cPort));
}

ErrorType I2c::txBlocking(const std::string &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    assert(std::nullopt != params.i2cDeviceAddress);
    assert(std::nullopt != params.i2cRegisterAddress);

    ErrorType error = ErrorType::Failure;
    const PeripheralNumber peripheral = i2cParams().hardwareConfig.peripheral;
    const I2cTypes::Mode mode = i2cParams().hardwareConfig.mode;
    i2c_port_t i2cPort = toEspPort(peripheral, error);
    if (ErrorType::Success != error) {
        return error;
    }

    //Append the register address to the beginning of the data.
    //TODO: 10-bit addressing?
    std::string writeData(data.size() + 1, 0);
    writeData.resize(0);
    writeData.push_back(static_cast<char>(params.i2cRegisterAddress.value()));
    writeData.append(data);

#if I2C_ESP_MODULE_DEBUG
    PLT_LOGI(TAG, "Tx");
    PLT_LOG_BUFFER_HEXDUMP(TAG, writeData.data(), writeData.size(), LogType::Info);
#endif

    if (I2cTypes::Mode::Controller == mode) {
        Ticks timeToWait;
        OperatingSystem::Instance().millisecondsToTicks(timeout, timeToWait);
        error = fromPlatformError(i2c_master_write_to_device(i2cPort, params.i2cDeviceAddress.value(), reinterpret_cast<const uint8_t *>(writeData.data()), writeData.size(), timeToWait));
    }
    else if (I2cTypes::Mode::Target == mode) {
        error = ErrorType::NotImplemented;
    }
    else {
        error = ErrorType::NotSupported;
    }

    return error;
}

ErrorType I2c::rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    assert(std::nullopt != params.i2cDeviceAddress);
    assert(std::nullopt != params.i2cRegisterAddress);

    ErrorType error = ErrorType::Failure;
    const PeripheralNumber peripheral = i2cParams().hardwareConfig.peripheral;
    const I2cTypes::Mode mode = i2cParams().hardwareConfig.mode;
    i2c_port_t i2cPort = toEspPort(peripheral, error);
    if (ErrorType::Failure == error) {
        return error;
    }

    if (I2cTypes::Mode::Controller == mode) {
        error = fromPlatformError(i2c_master_write_read_device(i2cPort, params.i2cDeviceAddress.value(), &params.i2cRegisterAddress.value(), 1, reinterpret_cast<uint8_t *>(buffer.data()), buffer.size(), timeout));
    }
    else if (I2cTypes::Mode::Target == mode) {
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

ErrorType I2c::txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType I2c::rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType I2c::flushRxBuffer() {
    return ErrorType::NotSupported;
}