#include "SpiModule.hpp"

ErrorType Spi::init() {
    return ErrorType::NotImplemented;
}
ErrorType Spi::deinit() {
    return ErrorType::NotImplemented;
}
ErrorType Spi::txBlocking(const std::string &data, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType Spi::txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType Spi::rxBlocking(std::string &buffer, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType Spi::rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType Spi::Spi::flushRxBuffer() {
    return ErrorType::NotImplemented;
}
ErrorType Spi::setHardwareConfig() {
    return ErrorType::NotImplemented;
}
ErrorType Spi::setDriverConfig() {
    return ErrorType::NotImplemented;
}
ErrorType Spi::setFirmwareConfig() {
    return ErrorType::NotImplemented;
}