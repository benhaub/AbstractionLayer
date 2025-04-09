#ifndef __I2C_MODULE_HPP__
#define __I2C_MODULE_HPP__

//AbstractionLayer
#include "I2cAbstraction.hpp"
//ESP
#include "driver/i2c.h"

class I2c : public I2cAbstraction {

    public:
    I2c() : I2cAbstraction() {}
    ~I2c() = default;

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType setHardwareConfig(const PeripheralNumber peripheral, const I2cConfig::Mode mode, const I2cConfig::Speed speed, const PinNumber sda, const bool sdaPullup, const PinNumber scl, const bool sclPullup) override;
    ErrorType setInterruptConfig(const bool arbitrationLost, const bool nackDetected, const bool sclLowTimeout, const bool stopDetect, const bool receiveFifoOverflow, const bool transmitFifoOverflow) override;
    ErrorType txBlocking(const std::string &data, uint8_t deviceAddress, uint8_t registerAddress, const Milliseconds timeout) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> data, uint8_t deviceAddress, uint8_t registerAddress, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType rxBlocking(std::string &buffer, uint8_t deviceAddress, uint8_t registerAddress, const Milliseconds timeout) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, uint8_t deviceAddress, uint8_t registerAddress, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;

    ErrorType flushRxBuffer() override;

    i2c_mode_t toEspMode(I2cConfig::Mode mode, ErrorType &error) {
        error = ErrorType::Success;

        switch (mode) {
            case I2cConfig::Mode::Controller:
                return I2C_MODE_MASTER;
            case I2cConfig::Mode::Target:
                return I2C_MODE_SLAVE;
            case I2cConfig::Mode::Unknown:
            default:
                error = ErrorType::NotSupported;
        }

        return I2C_MODE_MAX;
    }

    gpio_num_t toEspPinNumber(const PinNumber pin) {
        return static_cast<gpio_num_t>(pin);
    }

    Hertz toEspClockSpeed(const I2cConfig::Speed speed, ErrorType &error) {
        error = ErrorType::Success;

        switch (speed) {
            case I2cConfig::Speed::Standard:
            case I2cConfig::Speed::Fast:
            case I2cConfig::Speed::FastPlus:
            case I2cConfig::Speed::HighSpeed:
            case I2cConfig::Speed::UltraFast:
                return static_cast<Hertz>(speed);
            case I2cConfig::Speed::Unknown:
            default:
                error = ErrorType::NotSupported;
        }

        return 0;
    }

    i2c_port_t toEspPort(const PeripheralNumber peripheral, ErrorType &error) {
        error = ErrorType::Success;

        switch (peripheral) {
            case PeripheralNumber::Zero:
                return I2C_NUM_0;
            case PeripheralNumber::One:
                return I2C_NUM_1;
            case PeripheralNumber::Two:
            case PeripheralNumber::Three:
            case PeripheralNumber::Four:
            case PeripheralNumber::Five:
            case PeripheralNumber::Unknown:
            default:
                error = ErrorType::NotSupported;
        }

        return I2C_NUM_MAX;
    }
};

#endif //__I2C_MODULE_HPP__