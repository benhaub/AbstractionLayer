//Modules
#include "UartModule.hpp"
//ESP
#include "hal/uart_hal.h"
//C++
#include <string>

ErrorType Uart::init() {
    ErrorType error;

    //Did you call all of the config functions?
    assert(PeripheralNumber::Unknown != peripheralNumber());

    error = fromPlatformError(uart_driver_install(toEspPeripheralNumber(this->peripheralNumber(), error), receiveBufferSize(), transmitBufferSize(), 0, nullptr, 0));
    if (ErrorType::Success != error) {
        return error;
    }

    return error;
}

ErrorType Uart::deinit() {
    ErrorType error;
    return fromPlatformError(uart_driver_delete(toEspPeripheralNumber(this->peripheralNumber(), error)));
}

ErrorType Uart::txBlocking(const std::string &data, Milliseconds timeout) {
    ErrorType error;

    if (-1 != uart_write_bytes(toEspPeripheralNumber(this->peripheralNumber(), error), data.data(), data.size())) {
        return ErrorType::Success;
    }

    return ErrorType::Failure;
}

ErrorType Uart::txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

//Supports both a terminating byte read and a total byte read.
ErrorType Uart::rxBlocking(std::string &buffer, const Milliseconds timeout) {
    ErrorType error;
    esp_err_t result;
    constexpr Bytes toReadAtOnce = 16;
    Bytes readInThisFrame = 0;
    std::string readBuffer(toReadAtOnce, '\0');

    if (terminatingByte() >= 0) {
        buffer.resize(0);
    }
    else {
        uart_port_t uartNumber = toEspPeripheralNumber(this->peripheralNumber(), error);
        if (ErrorType::Success != error) {
            return error;
        }

        result = uart_read_bytes(uartNumber, buffer.data(), buffer.size(), pdMS_TO_TICKS(timeout));
        if (result > 0) {
            buffer.resize(result);
            return ErrorType::Success;
        }

        buffer.resize(0);
        return ErrorType::Timeout;
    }

    bool bufferHasRoom = readInThisFrame < buffer.capacity();

    while (bufferHasRoom) {
        uart_port_t uartNumber = toEspPeripheralNumber(this->peripheralNumber(), error);
        if (ErrorType::Success != error) {
            return error;
        }

        result = uart_read_bytes(uartNumber, readBuffer.data(), readBuffer.capacity(), pdMS_TO_TICKS(timeout));
        if (result <= 0) {
            return ErrorType::Failure;
        }

        readInThisFrame += readBuffer.size();

        buffer.append(readBuffer);

        if (terminatingByte() >= 0) {
            const size_t indexOfTerminatingByte = buffer.find(static_cast<char>(terminatingByte()), readInThisFrame - readBuffer.capacity());
            const bool terminatingByteFound = std::string::npos != indexOfTerminatingByte;

            if (terminatingByteFound) {
                buffer.resize(indexOfTerminatingByte + 1);
                return ErrorType::Success;
            }
        }

        bufferHasRoom = readInThisFrame < buffer.capacity();
    }

    return ErrorType::Failure;
}

ErrorType Uart::rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    auto rx = [this, callback](std::shared_ptr<std::string> buffer, Bytes size) -> ErrorType {
        ErrorType error = ErrorType::Failure;
        const Milliseconds timeout = 1000;

        error = rxBlocking(*(buffer.get()), timeout);

        if (nullptr != callback) {
            callback(error, buffer);
        }

        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(rx, buffer, buffer->size()));
    return addEvent(event);
}

ErrorType Uart::flushRxBuffer() {
    ErrorType error;

    esp_err_t err = uart_flush_input(toEspPeripheralNumber(this->peripheralNumber(), error));

    if (ErrorType::Success != error) {
        return error;
    }
    else {
        return fromPlatformError(err);
    }
}

ErrorType Uart::setHardwareConfig(int32_t txNumber, int32_t rxNumber, int32_t rtsNumber, int32_t ctsNumber, PeripheralNumber peripheralNumber) {
    ErrorType error;
    _txNumber = txNumber;
    _rxNumber = rxNumber;
    _rtsNumber = rtsNumber;
    _ctsNumber = ctsNumber;
    _peripheralNumber = peripheralNumber;

    uart_port_t uartPort = toEspPeripheralNumber(peripheralNumber, error);
    
    if (error != ErrorType::Success) {
        return ErrorType::InvalidParameter;
    }

    if (ESP_OK != uart_set_pin(uartPort, txNumber, rxNumber, rtsNumber, ctsNumber)) {
        return ErrorType::InvalidParameter;
    }

    return ErrorType::Success;
}

ErrorType Uart::setDriverConfig(uint32_t baudRate, uint8_t dataBits, char parity, uint8_t stopBits, UartTypes::FlowControl flowControl) {
    _baudRate = baudRate;
    _dataBits = dataBits;
    _parity = parity;
    _stopBits = stopBits;
    _flowControl = flowControl;

    uart_config_t uartConfig;
    ErrorType error;

    uartConfig.baud_rate = baudRate;
    uartConfig.rx_flow_ctrl_thresh = 0;
    uartConfig.source_clk = UART_SCLK_DEFAULT;

    uartConfig.data_bits = toEspWordLength(dataBits, error);
    if (ErrorType::Success != error) {
        return error;
    }
    uartConfig.parity = toEspUartParity(parity, error);
    if (ErrorType::Success != error) {
        return error;
    }
    uartConfig.stop_bits = toEspStopBits(stopBits, error);
    if (ErrorType::Success != error) {
        return error;
    }
    uartConfig.flow_ctrl = toEspFlowControl(flowControl, error);
    if (ErrorType::Success != error) {
        return error;
    }

    if (ESP_OK != uart_param_config(toEspPeripheralNumber(peripheralNumber(), error), &uartConfig)) {
        return ErrorType::InvalidParameter;
    }

    return ErrorType::Success;
}

ErrorType Uart::setFirmwareConfig(Bytes receiveBufferSize, Bytes transmitBufferSize, int8_t terminatingByte) {
    _receiveBufferSize = receiveBufferSize;
    _transmitBufferSize = transmitBufferSize;
    _terminatingByte = terminatingByte;

    return ErrorType::Success;
}

//Interrupts are enabled internally by uart_driver_install. It uses some weird macros to set thresholds and I'm not going to touch it.
//All interrupts are enabled by default, we will use the flags to evaluate just the ones that are set.
ErrorType Uart::setInterruptConfig(InterruptFlags interruptFlags, InterruptCallback interruptCallback) {
    _interruptFlags = interruptFlags;
    _interruptCallback = interruptCallback;

    return ErrorType::Success;
}