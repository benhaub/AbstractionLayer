#ifndef __UART_MODULE_HPP__
#define __UART_MODULE_HPP__

//AbstractionLayer
#include "UartAbstraction.hpp"
//Modules
#include "StorageModule.hpp"
#include "FileModule.hpp"

class Uart : public UartAbstraction {

    public:
    Uart() : UartAbstraction() {}

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType txBlocking(const std::string &data, const Milliseconds timeout) override;
    ErrorType rxBlocking(std::string &buffer, const Milliseconds timeout) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback = nullptr) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback = nullptr) override;
    ErrorType flushRxBuffer() override;

    ErrorType setHardwareConfig(PinNumber txNumber, PinNumber rxNumber, PinNumber rtsNumber, PinNumber ctsNumber, PeripheralNumber peripheralNumber) override;
    ErrorType setDriverConfig(uint32_t baudRate, uint8_t dataBits, char parity, uint8_t stopBits, UartTypes::FlowControl flowControl) override;
    ErrorType setFirmwareConfig(Bytes receiveBufferSize, Bytes transmitBufferSize, int8_t terminatingByte) override;
    ErrorType setInterruptConfig(InterruptFlags interruptFlags, InterruptCallback interruptCallback) override;

    private:
    std::unique_ptr<File> txBuffer;
    std::unique_ptr<File> rxBuffer;

    //The total number of bytes read so far from the file.
    Bytes _totalRead = 0;
};

#endif // __UART_MODULE_HPP__