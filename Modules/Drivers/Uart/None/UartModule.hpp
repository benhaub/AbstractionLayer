#ifndef __UART_MODULE_HPP__
#define __UART_MODULE_HPP__

#include "UartAbstraction.hpp"

class Uart : public UartAbstraction {
    public:
    Uart() : UartAbstraction() {}
    ~Uart() = default;

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType txBlocking(const std::string &data, const Milliseconds timeout) override;
    ErrorType rxBlocking(std::string &buffer, const Milliseconds timeout) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;
    ErrorType flushRxBuffer() override;

    ErrorType setHardwareConfig(PinNumber txNumber, PinNumber rxNumber, PinNumber rtsNumber, PinNumber ctsNumber, PeripheralNumber peripheralNumber) override;
    ErrorType setDriverConfig(uint32_t baudRate, uint8_t dataBits, char parity, uint8_t stopBits, UartConfig::FlowControl flowControl) override;
    ErrorType setFirmwareConfig(Bytes receiveBufferSize, Bytes transmitBufferSize, int8_t terminatingByte) override;
    ErrorType setInterruptConfig(const bool overrun, const bool breakError, const bool parityError, const bool framingError, const bool receiveTimeout, const bool transmitted, const bool received, const bool dsrModem, const bool dcdModem, const bool ctsModem, const bool riModem) override;
};

#endif // __UART_MODULE_HPP__