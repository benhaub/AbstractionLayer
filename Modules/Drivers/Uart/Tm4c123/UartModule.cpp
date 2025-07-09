//AbstractionLayer
#include "UartModule.hpp"
//TI
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

ErrorType Uart::init() {
    ErrorType error = ErrorType::PrerequisitesNotMet;

    if (PeripheralNumber::Unknown != peripheralNumber()) {
        const uint32_t tm4c123UartSysCtlPeripheralNumber = toTm4c123SysCtlPeripheralNumber(peripheralNumber(), error);
        if (ErrorType::Success == error) {
            SysCtlPeripheralEnable(tm4c123UartSysCtlPeripheralNumber);
            while(!SysCtlPeripheralReady(tm4c123UartSysCtlPeripheralNumber));
            Register tm4c123UartBaseRegister = toTm4c123PeripheralBaseRegister(peripheralNumber(), error);
            if (ErrorType::Success == error) {
                UARTConfigSetExpClk(reinterpret_cast<uint32_t>(tm4c123UartBaseRegister), SysCtlClockGet(), baudRate(), toTm4c123UartConfigBits(dataBits(), stopBits(), parity(), error));
                if (ErrorType::Success == error) {
                    UARTFlowControlSet(reinterpret_cast<uint32_t>(tm4c123UartBaseRegister), toTm4c123UartFlowControl(flowControl(), error));
                }
            }
        }
    }

    return error;
}

ErrorType Uart::deinit() {
    return ErrorType::NotImplemented;
}

ErrorType Uart::txBlocking(const std::string &data, const Milliseconds timeout) {
    ErrorType error;
    const Register tm4c123UartBaseRegister = toTm4c123PeripheralBaseRegister(peripheralNumber(), error);
    if (ErrorType::Success == error) {
        for (const char &c : data) {
            UARTCharPut(reinterpret_cast<uint32_t>(tm4c123UartBaseRegister), c);
        }
    }
    return error;
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
   txNumber = txNumber;
   rxNumber = rxNumber;
   rtsNumber = rtsNumber;
   ctsNumber = ctsNumber;
   peripheralNumber = peripheralNumber;
   return ErrorType::Success;
}

ErrorType Uart::setDriverConfig(uint32_t baudRate, uint8_t dataBits, char parity, uint8_t stopBits, UartTypes::FlowControl flowControl) {
    _baudRate = baudRate;
    _dataBits = dataBits;
    _parity = parity;
    _stopBits = stopBits;
    _flowControl = flowControl;
    return ErrorType::Success;
}

ErrorType Uart::setFirmwareConfig(Bytes receiveBufferSize, Bytes transmitBufferSize, int8_t terminatingByte) {
    _receiveBufferSize = receiveBufferSize;
    _transmitBufferSize = transmitBufferSize;
    _terminatingByte = terminatingByte;
    return ErrorType::Success;
}

ErrorType Uart::setInterruptConfig(InterruptFlags interruptFlags, InterruptCallback interruptCallback) {
    _interruptFlags = interruptFlags;
    _interruptCallback = interruptCallback;
    return ErrorType::Success;
}

uint32_t Uart::toTm4c123SysCtlPeripheralNumber(PeripheralNumber peripheralNumber, ErrorType &error) {
    error = ErrorType::Success;

    switch (peripheralNumber) {
        case PeripheralNumber::Zero:
            return SYSCTL_PERIPH_UART0;
        case PeripheralNumber::One:
            return SYSCTL_PERIPH_UART1;
        case PeripheralNumber::Two:
            return SYSCTL_PERIPH_UART2;
        case PeripheralNumber::Three:
            return SYSCTL_PERIPH_UART3;
        case PeripheralNumber::Four:
            return SYSCTL_PERIPH_UART4;
        case PeripheralNumber::Five:
            return SYSCTL_PERIPH_UART5;
        case PeripheralNumber::Six:
            return SYSCTL_PERIPH_UART6;
        case PeripheralNumber::Seven:
            return SYSCTL_PERIPH_UART7;
        default:
            error = ErrorType::InvalidParameter;
            return 0;
    }
}

Register Uart::toTm4c123PeripheralBaseRegister(PeripheralNumber peripheralNumber, ErrorType &error) {
    error = ErrorType::Success;

    switch (peripheralNumber) {
        case PeripheralNumber::Zero:
            return Register(UART0_BASE);
        case PeripheralNumber::One:
            return Register(UART1_BASE);
        case PeripheralNumber::Two:
            return Register(UART2_BASE);
        case PeripheralNumber::Three:
            return Register(UART3_BASE);
        case PeripheralNumber::Four:
            return Register(UART4_BASE);
        case PeripheralNumber::Five:
            return Register(UART5_BASE);
        case PeripheralNumber::Six:
            return Register(UART6_BASE);
        case PeripheralNumber::Seven:
            return Register(UART7_BASE);
        default:
            error = ErrorType::InvalidParameter;
            return 0;
    }
}

uint32_t Uart::toTm4c123UartConfigBits(const uint32_t dataBits, const uint32_t stopBits, const char parity, ErrorType &error) {
    error = ErrorType::Success;
    uint32_t configBits = 0;

    switch (dataBits) {
        case 5:
            configBits |= UART_CONFIG_WLEN_5;
            break;
        case 6:
            configBits |= UART_CONFIG_WLEN_6;
            break;
        case 7:
            configBits |= UART_CONFIG_WLEN_7;
            break;
        case 8:
            configBits |= UART_CONFIG_WLEN_8;
            break;
        default:
            error = ErrorType::InvalidParameter;
            return 0;
    }

    switch (stopBits) {
        case 1:
            configBits |= UART_CONFIG_STOP_ONE;
            break;
        case 2:
            configBits |= UART_CONFIG_STOP_TWO;
            break;
        default:
            error = ErrorType::InvalidParameter;
            return 0;
    }

    switch (parity) {
        case 'N':
        case 'n':
            configBits |= UART_CONFIG_PAR_NONE;
            break;
        case 'O':
        case 'o':
            configBits |= UART_CONFIG_PAR_ODD;
            break;
        case 'E':
        case 'e':
            configBits |= UART_CONFIG_PAR_EVEN;
            break;
        default:
            error = ErrorType::InvalidParameter;
            return 0;
    }

    return configBits;
}

uint32_t Uart::toTm4c123UartFlowControl(const UartTypes::FlowControl flowControl, ErrorType &error) {
    error = ErrorType::Success;

    switch (flowControl) {
        case UartTypes::FlowControl::Disable:
            return UART_FLOWCONTROL_NONE;
        case UartTypes::FlowControl::Rts:
            return UART_FLOWCONTROL_RX;
        case UartTypes::FlowControl::Cts:
            return UART_FLOWCONTROL_TX;
        case UartTypes::FlowControl::CtsRts:
            return UART_FLOWCONTROL_TX | UART_FLOWCONTROL_RX;
        default:
            error = ErrorType::InvalidParameter;
            return UART_FLOWCONTROL_NONE;
    }
}
