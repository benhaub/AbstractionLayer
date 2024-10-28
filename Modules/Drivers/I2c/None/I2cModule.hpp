#ifndef __I2C_MODULE_HPP__
#define __I2C_MOUDLE_HPP__

#include "I2cAbstraction.hpp"

class I2cModule : public I2cAbstraction {
    I2cModule() = default;
    ~I2cModule() = default;

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType setHardwareConfig(const I2cConfig::PeripheralNumber peripharal, const I2cConfig::Mode mode, const I2cConfig::Speed speed, const PinNumber sda, const bool sdaPullUp, const PinNumber scl, const bool sclPullUp) override;

    ErrorType txBlocking(const std::string &data, const Milliseconds timeout) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback = nullptr) override;
    ErrorType rxBlocking(std::string &buffer, const Milliseconds timeout) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback = nullptr) override;
    ErrorType flushRxBuffer() override;
};

#endif //__I2C_MODULE_HPP__