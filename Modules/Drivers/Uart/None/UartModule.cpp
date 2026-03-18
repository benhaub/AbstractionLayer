#include "UartModule.hpp"

ErrorType Uart::init() {
    return ErrorType::NotImplemented;
}

ErrorType Uart::deinit() {
    return ErrorType::NotImplemented;
}

ErrorType Uart::txBlocking(const StaticString::Container &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return ErrorType::NotImplemented;
}
ErrorType Uart::txBlocking(std::string_view data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return ErrorType::NotImplemented;
}
ErrorType Uart::txBlocking(const char *data, const size_t size, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Uart::rxBlocking(StaticString::Container &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return ErrorType::NotImplemented;
}
ErrorType Uart::rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return ErrorType::NotImplemented;
}
ErrorType Uart::rxBlocking(char *buffer, const size_t bufferSize, size_t &bytesRead, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Uart::txNonBlocking(const std::shared_ptr<StaticString::Container> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotAvailable;
}
ErrorType Uart::txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotAvailable;
}

ErrorType Uart::rxNonBlocking(std::shared_ptr<StaticString::Container> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<StaticString::Container> buffer)> callback) {
    return ErrorType::NotAvailable;
}
ErrorType Uart::rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotAvailable;
}

ErrorType Uart::flushRxBuffer() {
    return ErrorType::NotImplemented;
}