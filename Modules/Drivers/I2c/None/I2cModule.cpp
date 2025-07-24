#include "I2cModule.hpp"

ErrorType I2c::init() {
    return ErrorType::NotImplemented;
}
ErrorType I2c::deinit() {
    return ErrorType::NotImplemented;
}
ErrorType I2c::txBlocking(const std::string &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::flushRxBuffer() {
    return ErrorType::NotImplemented;
}