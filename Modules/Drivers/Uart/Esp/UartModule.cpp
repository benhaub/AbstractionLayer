//Modules
#include "UartModule.hpp"
//ESP
#include "hal/uart_hal.h"

ErrorType Uart::init() {
    ErrorType error;
    const uart_port_t uartPort = toEspPeripheralNumber(uartParams().hardwareConfig.peripheralNumber, error);

    if (ErrorType::Success == error) {

        if (ESP_OK == uart_driver_install(uartPort, uartParams().firmwareConfig.receiveBufferSize, uartParams().firmwareConfig.transmitBufferSize, 0, nullptr, 0)) {
            uart_config_t uartConfig;
            uartConfig.baud_rate = uartParams().driverConfig.baudRate;
            uartConfig.rx_flow_ctrl_thresh = 0;
            uartConfig.source_clk = UART_SCLK_DEFAULT;
            uartConfig.data_bits = toEspWordLength(uartParams().driverConfig.dataBits, error);

            uartConfig.parity = toEspUartParity(uartParams().driverConfig.parity, error);

            if (ErrorType::Success == error) {
                uartConfig.stop_bits = toEspStopBits(uartParams().driverConfig.stopBits, error);

                if (ErrorType::Success == error) {
                    uartConfig.flow_ctrl = toEspFlowControl(uartParams().driverConfig.flowControl, error);

                    uartConfig.flags.allow_pd = 0;
                    uartConfig.flags.backup_before_sleep = 0;

                    if (ESP_OK == uart_param_config(uartPort, &uartConfig)) {

                        if (ESP_OK == uart_set_pin(uartPort, uartParams().hardwareConfig.tx, uartParams().hardwareConfig.rx, uartParams().hardwareConfig.rts, uartParams().hardwareConfig.cts)) {
                            error = ErrorType::Success;
                        }
                    }
                }
            }
        }
    }

    return error;
}

ErrorType Uart::deinit() {
    ErrorType error = ErrorType::Failure;
    const uart_port_t uartPort = toEspPeripheralNumber(uartParams().hardwareConfig.peripheralNumber, error);

    if (ErrorType::Success == error) {
        error = fromPlatformError(uart_driver_delete(uartPort));
    }

    return error;
}

ErrorType Uart::txBlocking(const StaticString::Container &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return txBlocking(data->c_str(), data->size(), timeout);
}
ErrorType Uart::txBlocking(const std::string &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return txBlocking(data.c_str(), data.size(), timeout);
}
ErrorType Uart::txBlocking(const char *data, const size_t size, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;
    const uart_port_t uartPort = toEspPeripheralNumber(uartParams().hardwareConfig.peripheralNumber, error);

    if (ErrorType::Success == error) {
        if (-1 != uart_write_bytes(uartPort, data, size)) {
            error = ErrorType::Success;
        }
    }

    return error;
}

ErrorType Uart::rxBlocking(StaticString::Container &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    ErrorType error = ErrorType::Failure;
    size_t bytesRead = 0;

    if (ErrorType::Success == (error = rxBlocking(&buffer[0], buffer->size(), bytesRead, timeout))) {
        buffer->resize(bytesRead);
    }

    return error;
}
ErrorType Uart::rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    ErrorType error = ErrorType::Failure;
    size_t bytesRead = 0;

    if (ErrorType::Success == (error = rxBlocking(&buffer[0], buffer.size(), bytesRead, timeout))) {
        buffer.resize(bytesRead);
    }

    return error;
}
ErrorType Uart::rxBlocking(char *buffer, const size_t bufferSize, size_t &bytesRead, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;
    const uart_port_t uartPort = toEspPeripheralNumber(uartParams().hardwareConfig.peripheralNumber, error);

    if (ErrorType::Success == error) {
        const esp_err_t result = uart_read_bytes(uartPort, buffer, bufferSize, pdMS_TO_TICKS(timeout));
        if (result > 0) {
            bytesRead = result;
            error = ErrorType::Success;
        }
        else if (0 == result) {
            error = ErrorType::Timeout;
        }
    }

    return error;
}

ErrorType Uart::txNonBlocking(const std::shared_ptr<StaticString::Container> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType Uart::txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType Uart::rxNonBlocking(std::shared_ptr<StaticString::Container> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<StaticString::Container> buffer)> callback) {
    auto rx = [&, callback, buffer]() -> ErrorType {
        ErrorType error = ErrorType::Failure;
        const Milliseconds timeout = 1000;

        error = rxBlocking(*buffer, timeout, params);

        if (nullptr != callback) {
            callback(error, buffer);
        }

        return error;
    };

    EventQueue::Event event = EventQueue::Event(rx);
    return addEvent(event);
}
ErrorType Uart::rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    auto rx = [&, callback, buffer]() -> ErrorType {
        ErrorType error = ErrorType::Failure;
        const Milliseconds timeout = 1000;

        error = rxBlocking(*buffer, timeout, params);

        if (nullptr != callback) {
            callback(error, buffer);
        }

        return error;
    };

    EventQueue::Event event = EventQueue::Event(rx);
    return addEvent(event);
}

ErrorType Uart::flushRxBuffer() {
    ErrorType error = ErrorType::Failure;
    const uart_port_t uartPort = toEspPeripheralNumber(uartParams().hardwareConfig.peripheralNumber, error);

    if (ErrorType::Success == error) {
        error = fromPlatformError(uart_flush_input(uartPort));
    }

    return error;
}