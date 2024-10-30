#include "I2cModule.hpp"

ErrorType I2c::init() {
    return ErrorType::NotImplemented;
}
ErrorType I2c::deinit() {
    return ErrorType::NotImplemented;
}
ErrorType I2c::setHardwareConfig(const I2cConfig::PeripheralNumber peripheral, const I2cConfig::Mode mode, const I2cConfig::Speed speed, const PinNumber sda, const bool sdaPullup, const PinNumber scl, const bool sclPullup) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::txBlocking(const std::string &data, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::txNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::rxBlocking(std::string &buffer, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::rxNonBlocking(std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::flushRxBuffer() {
    return ErrorType::NotImplemented;
}