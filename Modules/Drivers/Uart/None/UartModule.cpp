#include "UartModule.hpp"

ErrorType Uart::init() {
    return ErrorType::NotImplemented;
}

ErrorType Uart::deinit() {
    return ErrorType::NotImplemented;
}

ErrorType Uart::txBlocking(const std::string &data, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Uart::rxBlocking(std::string &buffer, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Uart::txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType Uart::rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType Uart::flushRxBuffer() {
    return ErrorType::NotImplemented;
}

ErrorType Uart::setHardwareConfig(PinNumber txNumber, PinNumber rxNumber, PinNumber rtsNumber, PinNumber ctsNumber, PeripheralNumber peripheralNumber) {
    return ErrorType::NotImplemented;
}

ErrorType Uart::setDriverConfig(uint32_t baudRate, uint8_t dataBits, char parity, uint8_t stopBits, UartTypes::FlowControl flowControl) {
    return ErrorType::NotImplemented;
}

ErrorType Uart::setFirmwareConfig(Bytes receiveBufferSize, Bytes transmitBufferSize, int8_t terminatingByte) {
    return ErrorType::NotImplemented;
}

ErrorType Uart::setInterruptConfig(InterruptFlags interruptFlags, InterruptCallback interruptCallback) {
    return ErrorType::NotImplemented;
}