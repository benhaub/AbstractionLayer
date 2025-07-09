#include "I2cModule.hpp"

ErrorType I2c::init() {
    return ErrorType::NotImplemented;
}
ErrorType I2c::deinit() {
    return ErrorType::NotImplemented;
}
ErrorType I2c::setHardwareConfig(const PeripheralNumber peripheral, const I2cTypes::Mode mode, const I2cTypes::Speed speed, const PinNumber sda, const bool sdaPullup, const PinNumber scl, const bool sclPullup) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::setInterruptConfig(const bool arbitrationLost, const bool nackDetected, const bool sclLowTimeout, const bool stopDetect, const bool receiveFifoOverflow, const bool transmitFifoOverflow) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::txBlocking(const std::string &data, uint8_t deviceAddress, uint8_t registerAddress, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::rxBlocking(std::string &buffer, uint8_t deviceAddress, uint8_t registerAddress, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::txNonBlocking(const std::shared_ptr<std::string> data, uint8_t deviceAddress, uint8_t registerAddress, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::rxNonBlocking(std::shared_ptr<std::string> buffer, uint8_t deviceAddress, uint8_t registerAddress, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType I2c::flushRxBuffer() {
    return ErrorType::NotImplemented;
}