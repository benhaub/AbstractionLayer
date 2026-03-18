#ifndef __I2C_MODULE_HPP__
#define __I2C_MODULE_HPP__

//AbstractionLayer
#include "I2cAbstraction.hpp"
//ESP
#include "driver/i2c_master.h"
#include "driver/i2c_slave.h"

class I2c final : public I2cAbstraction {

    public:
    I2c() : I2cAbstraction() {}

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType txBlocking(const StaticString::Container &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) override;
    ErrorType txBlocking(std::string_view data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) override;
    ErrorType rxBlocking(StaticString::Container &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) override;
    ErrorType rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) override;
    ErrorType txNonBlocking(const std::shared_ptr<StaticString::Container> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType rxNonBlocking(std::shared_ptr<StaticString::Container> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<StaticString::Container> buffer)> callback) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;
    ErrorType flushRxBuffer() override;

    i2c_mode_t toEspMode(I2cTypes::Mode mode, ErrorType &error) {
        error = ErrorType::Success;

        switch (mode) {
            case I2cTypes::Mode::Controller:
                return I2C_MODE_MASTER;
            case I2cTypes::Mode::Target:
                return I2C_MODE_SLAVE;
            case I2cTypes::Mode::Unknown:
            default:
                error = ErrorType::NotSupported;
        }

        return I2C_MODE_MAX;
    }

    gpio_num_t toEspPinNumber(const PinNumber pin) {
        return static_cast<gpio_num_t>(pin);
    }

    Hertz toEspClockSpeed(const I2cTypes::Speed speed, ErrorType &error) {
        error = ErrorType::Success;

        switch (speed) {
            case I2cTypes::Speed::Standard:
            case I2cTypes::Speed::Fast:
            case I2cTypes::Speed::FastPlus:
            case I2cTypes::Speed::HighSpeed:
            case I2cTypes::Speed::UltraFast:
                return static_cast<Hertz>(speed);
            case I2cTypes::Speed::Unknown:
            default:
                error = ErrorType::NotSupported;
        }

        return 0;
    }

    i2c_port_num_t toEspPort(const PeripheralNumber peripheral, ErrorType &error) {
        error = ErrorType::Success;

        switch (peripheral) {
            case PeripheralNumber::Zero:
                return I2C_NUM_0;
            case PeripheralNumber::One:
#if SOC_HP_I2C_NUM >= 2
                return I2C_NUM_1;
#endif
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