#ifndef __SPI_MODULE_HPP__
#define __SPI_MODULE_HPP__

#include "SpiAbstraction.hpp"

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
    ErrorType txBlocking(const char *data, const size_t size, const Milliseconds timeout);
    ErrorType rxBlocking(char *buffer, const size_t bufferSize, size_t &bytesRead, const Milliseconds timeout);
};

#endif // __SPI_MODULE_HPP__