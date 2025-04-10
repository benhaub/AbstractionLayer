#ifndef __UART_MODULE_HPP__
#define __UART_MODULE_HPP__

//AbstractionLayer
#include "UartAbstraction.hpp"
//FreeRtos
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
//ESP
#include "driver/uart.h"

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

    private:
    uart_port_t toEspPeripheralNumber(PeripheralNumber peripheralNumber, ErrorType &error) {
        switch (peripheralNumber) {
            case PeripheralNumber::Zero:
                error = ErrorType::Success;
                return UART_NUM_0;
            case PeripheralNumber::One:
                error = ErrorType::Success;
                return UART_NUM_1;
            case PeripheralNumber::Two:
                error = ErrorType::Success;
                return UART_NUM_2;
            default:
                error = ErrorType::InvalidParameter;
                return UART_NUM_0;
        }
    }

    uart_word_length_t toEspWordLength(uint8_t dataBits, ErrorType &error) {
        error = ErrorType::Success;

        switch (dataBits) {
            case 8:
                return UART_DATA_8_BITS;
            case 7:
                return UART_DATA_7_BITS;
            case 6:
                return UART_DATA_6_BITS;
            case 5:
                return UART_DATA_5_BITS;
            default:
                error = ErrorType::InvalidParameter;
        }

        return UART_DATA_8_BITS;
    }

    uart_parity_t toEspUartParity(char parity, ErrorType &error) {
        error = ErrorType::Success;

        switch (parity) {
            case 'N':
                return UART_PARITY_DISABLE;
            case 'E':
                return UART_PARITY_EVEN;
            case 'O':
                return UART_PARITY_ODD;
            default:
                error = ErrorType::InvalidParameter;
        }

        return UART_PARITY_DISABLE;
    }

    uart_stop_bits_t toEspStopBits(uint8_t stopBits, ErrorType &error) {
        error = ErrorType::Success;

        switch (stopBits) {
            case 1:
                return UART_STOP_BITS_1;
            case 2:
                return UART_STOP_BITS_2;
            default:
                error = ErrorType::InvalidParameter;
        }

        return UART_STOP_BITS_1;
    }

    uart_hw_flowcontrol_t toEspFlowControl(UartConfig::FlowControl flowControl, ErrorType &error) {
        error = ErrorType::Success;

        switch (flowControl) {
            case UartConfig::FlowControl::Disable:
                return UART_HW_FLOWCTRL_DISABLE;
            case UartConfig::FlowControl::Rts:
                return UART_HW_FLOWCTRL_RTS;
            case UartConfig::FlowControl::Cts:
                return UART_HW_FLOWCTRL_CTS;
            case UartConfig::FlowControl::CtsRts:
                return UART_HW_FLOWCTRL_CTS_RTS;
            default:
                error = ErrorType::InvalidParameter;
        }

        return UART_HW_FLOWCTRL_DISABLE;
    }
};

#endif // __UART_MODULE_HPP__
