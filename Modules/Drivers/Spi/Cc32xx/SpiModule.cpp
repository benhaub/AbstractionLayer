#include "SpiModule.hpp"
#include "ti/drivers/SPI.h"

ErrorType Spi::init() {
    SPI_init();
    return ErrorType::Success;
}

ErrorType Spi::deinit() {
    return ErrorType::NotImplemented;
}

ErrorType Spi::txBlocking(const StaticString::Container &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return ErrorType::NotImplemented;
}
ErrorType Spi::txBlocking(std::string_view data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return ErrorType::NotImplemented;
}
ErrorType Spi::txBlocking(const char *data, const size_t size, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Spi::rxBlocking(StaticString::Container &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return ErrorType::NotImplemented;
}
ErrorType Spi::rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return ErrorType::NotImplemented;
}
ErrorType Spi::rxBlocking(char *buffer, const size_t bufferSize, size_t &bytesRead, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Spi::txNonBlocking(const std::shared_ptr<StaticString::Container> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType Spi::txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType Spi::rxNonBlocking(std::shared_ptr<StaticString::Container> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<StaticString::Container> buffer)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType Spi::rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType Spi::Spi::flushRxBuffer() {
    return ErrorType::NotImplemented;
}