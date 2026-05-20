#ifndef __SPI_MODULE_HPP__
#define __SPI_MODULE_HPP__

//AbstractionLayer
#include "SpiAbstraction.hpp"
//LibMPSSE
#include "ftd2xx.h"
#include "libmpsse_spi.h"


class Spi : public SpiAbstraction {
    public:
    Spi() : SpiAbstraction() {}

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

    private:
    FT_HANDLE _handle = nullptr;

    uint32_t toLibMpsseChannelNumber(const PeripheralNumber peripheralNumber, ErrorType &error) {
        error = ErrorType::Success;
        return static_cast<uint32_t>(peripheralNumber);
    }

    uint32_t toLibMpsseConfigOptions(ErrorType &error) {
        error = ErrorType::Success;
        uint32_t configuration = 0;

        if (spiParams().driverConfig.format == SpiTypes::FrameFormat::Mode0) {
            configuration |= SPI_CONFIG_OPTION_MODE0;
        }
        else if (spiParams().driverConfig.format == SpiTypes::FrameFormat::Mode1) {
            configuration |= SPI_CONFIG_OPTION_MODE1;
        }
        else if (spiParams().driverConfig.format == SpiTypes::FrameFormat::Mode2) {
            configuration |= SPI_CONFIG_OPTION_MODE2;
        }
        else if (spiParams().driverConfig.format == SpiTypes::FrameFormat::Mode3) {
            configuration |= SPI_CONFIG_OPTION_MODE3;
        }
        else {
            error = ErrorType::InvalidParameter;
        }

        if (spiParams().driverConfig.chipSelectActiveLow) {
            configuration |= SPI_CONFIG_OPTION_CS_ACTIVELOW;
        }
        else {
            configuration |= SPI_CONFIG_OPTION_CS_ACTIVEHIGH;
        }

        return configuration;
    }

    ErrorType txBlocking(const char *data, const size_t size, const Milliseconds timeout);
    ErrorType rxBlocking(char *buffer, const size_t bufferSize, size_t &bytesRead, const Milliseconds timeout);
};

#endif // __SPI_MODULE_HPP__