#ifndef __I2C_MODULE_HPP__
#define __I2C_MODULE_HPP__

#include "I2cAbstraction.hpp"

class I2c final : public I2cAbstraction {

    public:
    I2c() : I2cAbstraction() {}

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType setHardwareConfig(const PeripheralNumber peripheral, const I2cTypes::Mode mode, const I2cTypes::Speed speed, const PinNumber sda, const bool sdaPullup, const PinNumber scl, const bool sclPullup) override;
    ErrorType setInterruptConfig(const bool arbitrationLost, const bool nackDetected, const bool sclLowTimeout, const bool stopDetect, const bool receiveFifoOverflow, const bool transmitFifoOverflow) override;

    ErrorType txBlocking(const std::string &data, uint8_t deviceAddress, uint8_t registerAddress, const Milliseconds timeout) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> data, uint8_t deviceAddress, uint8_t registerAddress, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType rxBlocking(std::string &buffer, uint8_t deviceAddress, uint8_t registerAddress, const Milliseconds timeout) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, uint8_t deviceAddress, uint8_t registerAddress, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;
    ErrorType flushRxBuffer() override;
};

#endif //__I2C_MODULE_HPP__