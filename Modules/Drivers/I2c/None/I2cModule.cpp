#include "I2cModule.hpp"

ErrorType I2cModule::init() {
    return ErrorType::NotImplemented;
}
ErrorType I2cModule::deinit() {
    return ErrorType::NotImplemented;
}
ErrorType setHardwareConfig(const I2cConfig::PeripheralNumber peripharal, const I2cConfig::Mode mode, const I2cConfig::Speed speed, const PinNumber sda, const bool sdaPullUp, const PinNumber scl, const bool sclPullUp) {
    return ErrorType::NotImplemented;
}
ErrorType I2cModule::txBlocking(const std::string &data, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType I2cModule::txNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType I2cModule::rxBlocking(std::string &buffer, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType I2cModule::rxNonBlocking(std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType I2cModule::flushRxBuffer() {
    return ErrorType::NotImplemented;
}