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

ErrorType Uart::txBlocking(const std::string &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    ErrorType error = ErrorType::Failure;
    const uart_port_t uartPort = toEspPeripheralNumber(uartParams().hardwareConfig.peripheralNumber, error);

    if (ErrorType::Success == error) {
        if (-1 != uart_write_bytes(uartPort, data.data(), data.size())) {
            error = ErrorType::Success;
        }
    }

    return error;
}

ErrorType Uart::rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    ErrorType error = ErrorType::Failure;
    const uart_port_t uartPort = toEspPeripheralNumber(uartParams().hardwareConfig.peripheralNumber, error);

    if (ErrorType::Success == error) {
        const esp_err_t result = uart_read_bytes(uartPort, &buffer[0], buffer.size(), pdMS_TO_TICKS(timeout));
        if (result > 0) {
            buffer.resize(result);
            error = ErrorType::Success;
        }
        else if (0 == result) {
            error = ErrorType::Timeout;
        }
    }

    return error;
}

ErrorType Uart::txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
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