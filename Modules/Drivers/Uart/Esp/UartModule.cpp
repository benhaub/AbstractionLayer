//Modules
#include "UartModule.hpp"
//ESP
#include "hal/uart_hal.h"
//C++
#include <string>

ErrorType Uart::init() {
    ErrorType error;

    uart_port_t uartPort = toEspPeripheralNumber(uartParams().hardwareConfig.peripheralNumber, error);
    if (ErrorType::Success == error) {
        if (ESP_OK == uart_set_pin(uartPort, uartParams().hardwareConfig.tx, uartParams().hardwareConfig.rx, uartParams().hardwareConfig.rts, uartParams().hardwareConfig.cts)) {
            uart_config_t uartConfig;
            uartConfig.baud_rate = uartParams().driverConfig.baudRate;
            uartConfig.rx_flow_ctrl_thresh = 0;
            uartConfig.source_clk = UART_SCLK_DEFAULT;

            uartConfig.data_bits = toEspWordLength(uartParams().driverConfig.dataBits, error);
            if (ErrorType::Success == error) {
                uartConfig.parity = toEspUartParity(uartParams().driverConfig.parity, error);

                if (ErrorType::Success == error) {
                    uartConfig.stop_bits = toEspStopBits(uartParams().driverConfig.stopBits, error);

                    if (ErrorType::Success == error) {
                        uartConfig.flow_ctrl = toEspFlowControl(uartParams().driverConfig.flowControl, error);

                        if (ErrorType::Success == error) {
                            if (ESP_OK == uart_param_config(toEspPeripheralNumber(uartParams().hardwareConfig.peripheralNumber, error), &uartConfig)) {
                                error = fromPlatformError(uart_driver_install(toEspPeripheralNumber(uartParams().hardwareConfig.peripheralNumber, error), uartParams().firmwareConfig.receiveBufferSize, uartParams().firmwareConfig.transmitBufferSize, 0, nullptr, 0));
                            }
                        }
                    }
                }
            }
        }
    }

    return error;
}

ErrorType Uart::deinit() {
    ErrorType error;
    return fromPlatformError(uart_driver_delete(toEspPeripheralNumber(uartParams().hardwareConfig.peripheralNumber, error)));
}

ErrorType Uart::txBlocking(const std::string &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    ErrorType error;
    const PeripheralNumber peripheralNumber = uartParams().hardwareConfig.peripheralNumber;

    if (-1 != uart_write_bytes(toEspPeripheralNumber(peripheralNumber, error), data.data(), data.size())) {
        return ErrorType::Success;
    }

    return ErrorType::Failure;
}

ErrorType Uart::rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    ErrorType error;
    esp_err_t result;
    constexpr Bytes toReadAtOnce = 16;
    std::string readBuffer(toReadAtOnce, '\0');
    const PeripheralNumber peripheralNumber = uartParams().hardwareConfig.peripheralNumber;

    uart_port_t uartNumber = toEspPeripheralNumber(peripheralNumber, error);
    if (ErrorType::Success != error) {
        return error;
    }

    result = uart_read_bytes(uartNumber, buffer.data(), buffer.size(), pdMS_TO_TICKS(timeout));
    if (result > 0) {
        buffer.resize(result);
        return ErrorType::Success;
    }

    return ErrorType::Timeout;
}

ErrorType Uart::txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType Uart::rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    auto rx = [&, callback](std::shared_ptr<std::string> buffer, Bytes size) -> ErrorType {
        ErrorType error = ErrorType::Failure;
        const Milliseconds timeout = 1000;

        error = rxBlocking(*(buffer.get()), timeout, params);

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
    const PeripheralNumber peripheralNumber = uartParams().hardwareConfig.peripheralNumber;

    esp_err_t err = uart_flush_input(toEspPeripheralNumber(peripheralNumber, error));

    if (ErrorType::Success != error) {
        return error;
    }
    else {
        return fromPlatformError(err);
    }
}