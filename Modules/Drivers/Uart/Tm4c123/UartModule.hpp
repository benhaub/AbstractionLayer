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
    ErrorType txBlocking(const std::string &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) override;
    ErrorType rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;
    ErrorType flushRxBuffer() override;

    private:
    uint32_t toTm4c123SysCtlPeripheralNumber(PeripheralNumber peripheralNumber, ErrorType &error);
    Register toTm4c123PeripheralBaseRegister(PeripheralNumber peripheralNumber, ErrorType &error);
    uint32_t toTm4c123UartConfigBits(const uint32_t dataBits, const uint32_t stopBits, const char parity, ErrorType &error);
    uint32_t toTm4c123UartFlowControl(const UartTypes::FlowControl flowControl, ErrorType &error);
};

#endif // __UART_MODULE_HPP__