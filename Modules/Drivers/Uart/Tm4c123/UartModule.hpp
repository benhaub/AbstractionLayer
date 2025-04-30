#ifndef __UART_MODULE_HPP__
#define __UART_MODULE_HPP__

//AbstractionLayer
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
    ErrorType setDriverConfig(uint32_t baudRate, uint8_t dataBits, char parity, uint8_t stopBits, UartTypes::FlowControl flowControl) override;
    ErrorType setFirmwareConfig(Bytes receiveBufferSize, Bytes transmitBufferSize, int8_t terminatingByte) override;
    ErrorType setInterruptConfig(InterruptFlags interruptFlags, InterruptCallback interruptCallback) override;

    private:
    uint32_t toTm4c123SysCtlPeripheralNumber(PeripheralNumber peripheralNumber, ErrorType &error);
    Register toTm4c123PeripheralBaseRegister(PeripheralNumber peripheralNumber, ErrorType &error);
    uint32_t toTm4c123UartConfigBits(const uint32_t dataBits, const uint32_t stopBits, const char parity, ErrorType &error);
    uint32_t toTm4c123UartFlowControl(const UartTypes::FlowControl flowControl, ErrorType &error);
};

#endif // __UART_MODULE_HPP__