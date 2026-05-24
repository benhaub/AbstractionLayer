#ifndef __SPI_MODULE_HPP__
#define __SPI_MODULE_HPP__

#include "SpiAbstraction.hpp"
#include "GpioModule.hpp"

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
    /// @brief The GPIO for chip select when using GPIO mode
    std::optional<Gpio> _chipSelect;

    ErrorType txBlocking(const char *data, const size_t size, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params);
    ErrorType rxBlocking(char *buffer, const size_t bufferSize, size_t &bytesRead, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params);

    uint32_t toTm4cSysCtlPeripheralNumber(const PeripheralNumber peripheralNumber, ErrorType &error);
    Register toTm4cPeripheralBaseRegister(const PeripheralNumber peripheralNumber, ErrorType &error);
    uint32_t toTm4cFrameFormat(const SpiTypes::FrameFormat format, ErrorType &error);
    uint32_t toTm4cDataSize(const SpiTypes::DataSize dataSize, ErrorType &error);
};

#endif // __SPI_MODULE_HPP__