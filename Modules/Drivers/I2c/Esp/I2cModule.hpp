#ifndef __I2C_MODULE_HPP__
#define __I2C_MOUDLE_HPP__

//AbstractionLayer
#include "I2cAbstraction.hpp"
//ESP
#include "driver/i2c.h"

class I2cModule : public I2cAbstraction {
    I2cModule() = default;
    ~I2cModule() = default;

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType setHardwareConfig(const I2cConfig::PeripheralNumber peripheral, const I2cConfig::Mode mode, const I2cConfig::Speed speed, const PinNumber sda, const bool sdaPullup, const PinNumber scl, const bool sclPullup) override;

    ErrorType txBlocking(const std::string &data, const Milliseconds timeout) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback = nullptr) override;
    ErrorType rxBlocking(std::string &buffer, const Milliseconds timeout) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback = nullptr) override;
    ErrorType flushRxBuffer() override;

    /**
     * @brief currentWriteAddressConst
     * @returns A const reference to the write address
     */
    const uint16_t &currentWriteAddressConst() { return _currentWriteAddress; }
    /**
     * @brief currentWriteAddress
     * @returns A mutable reference the the write address
     */
    uint16_t &currentWriteAddress() { return _currentWriteAddress; }
    /**
     * @brief currentReadAddressConst
     * @returns A const reference to the read address
     */
    const uint16_t &currentReadAddressConst() { return _currentWriteAddress; }
    /**
     * @brief currentReadAddress
     * @returns A mutable reference the the read address
     */
    uint16_t &currentReadAddress() { return _currentReadAddress; }

    private:
    uint16_t _currentWriteAddress;
    uint16_t _currentReadAddress;

    i2c_mode_t toEspMode(I2cConfig::Mode mode, ErrorType &error) {
        error = ErrorType::Success;

        switch (mode) {
            case I2cConfig::Mode::Master:
            case I2cConfig::Mode::Slave:
            case I2cConfig::Mode::Unknown:
            default:
                error = ErrorType::NotSupported;
        }

        return I2C_MODE_MAX;
    }

    gpio_num_t toEspPinNumber(const PinNumber pin) {
        return static_cast<gpio_num_t>(pin);
    }

    uint32_t toEspClockSpeed(const I2cConfig::Speed speed, ErrorType &error) {
        error = ErrorType::Success;

        switch (speed) {
            case I2cConfig::Speed::Unknown:
            case I2cConfig::Speed::Standard:
            case I2cConfig::Speed::Fast:
            case I2cConfig::Speed::FastPlus:
            case I2cConfig::Speed::HighSpeed:
            case I2cConfig::Speed::UltraFast:
            default:
                error = ErrorType::NotSupported;
        }

        return 0;
    }

    i2c_port_t toEspPort(const I2cConfig::PeripheralNumber peripheral, ErrorType &error) {
        error = ErrorType::Success;

        switch (peripheral) {
            case I2cConfig::PeripheralNumber::Zero:
                return I2C_NUM_0;
            case I2cConfig::PeripheralNumber::One:
                return I2C_NUM_1;
            case I2cConfig::PeripheralNumber::Two:
            case I2cConfig::PeripheralNumber::Three:
            case I2cConfig::PeripheralNumber::Four:
            case I2cConfig::PeripheralNumber::Five:
            case I2cConfig::PeripheralNumber::Unknown:
            default:
                error = ErrorType::NotSupported;
        }

        return I2C_NUM_MAX;
    }
};

#endif //__I2C_MODULE_HPP__