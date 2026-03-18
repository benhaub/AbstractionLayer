//AbstractionLayer
#include "CellularModule.hpp"
//ESP
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "cxx_include/esp_modem_api.hpp"
#include "cxx_include/esp_modem_dce_factory.hpp"

//Global so that the handler can access it. Otherwise would have to make it a public data member.
static EventGroupHandle_t cellularEventGroup;

ErrorType Cellular::init() {
    ErrorType error = ErrorType::NotSupported;
    cellularEventGroup = xEventGroupCreate();

    if (ESP_OK == esp_netif_init()) {
        const bool isNotCriticalErrror = (ESP_ERR_INVALID_STATE == esp_event_loop_create_default());

        if (isNotCriticalErrror) {

            if(IcCommunicationProtocolTypes::IcDevice::Uart == params().icDevice) {
                _dteConfig = ESP_MODEM_DTE_DEFAULT_CONFIG();
                _dteConfig.uart_config.tx_io_num = params().icCommTx;
                _dteConfig.uart_config.rx_io_num = params().icCommRx;
                _dteConfig.uart_config.rts_io_num = params().icCommRts;
                _dteConfig.uart_config.cts_io_num = params().icCommCts;
                _dteConfig.uart_config.rx_buffer_size = 1024;
                _dteConfig.uart_config.tx_buffer_size = 512;
                _dteConfig.uart_config.event_queue_size = 30;
                _dteConfig.task_stack_size = 4096;
                _dteConfig.task_priority = uxTaskPriorityGet(NULL);
                _dteConfig.dte_buffer_size = _dteConfig.uart_config.rx_buffer_size / 2;

                auto dte = esp_modem::create_uart_dte(&_dteConfig);

                if (nullptr != dte) {
                    _dceConfig = ESP_MODEM_DCE_DEFAULT_CONFIG(params().apn.data());
                    _dce = sock_dce::create(&_dceConfig, std::move(dte));
                }

                error = ErrorType::Success;
            }
        }
    }

    return error;
}

ErrorType Cellular::networkUp() {
    ErrorType error = ErrorType::Failure;

    if (!_initialized && _dce->init()) {
        error = ErrorType::Success;
    }
    else if (_dce->net_open() == esp_modem::command_result::OK) {
        error = ErrorType::Success;
    }

    return error;
}

ErrorType Cellular::networkDown() {
    ErrorType error = ErrorType::Failure;

    if (esp_modem::command_result::OK == _dce->net_close()) {
        error = ErrorType::Success;
    }

    return error;
}

ErrorType Cellular::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &sock, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;

    if (_dce->connect(std::string(hostname), port)) {
        error = ErrorType::Success;
    }

    return error;
}
ErrorType Cellular::disconnect(const Socket &socket) {
    ErrorType error = ErrorType::Failure;

    if (esp_modem::command_result::OK == _dce->tcp_close()) {
        error = ErrorType::Success;
    }

    return error;
}
ErrorType Cellular::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port, Socket &listenerSocket) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::acceptConnection(const Socket &listenerSocket, Socket &newSocket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::closeConnection(const Socket socket) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::transmit(std::string_view frame, const Socket socket, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;

    if (_dce->sync_send(frame.data(), frame.size(), timeout) > 0) {
        error = ErrorType::Success;
    }
    else {
        error = ErrorType::Failure;
    }

    return error;
}

ErrorType Cellular::receive(char *frameBuffer, const size_t bufferSize, const Socket socket, Bytes &read, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;

    if (_dce->sync_recv(frameBuffer, bufferSize, timeout) > 0) {
        error = ErrorType::Success;
    }
    else {
        error = ErrorType::Failure;
    }

    return error;
}

ErrorType Cellular::getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) {
    return ErrorType::NotAvailable;
}

ErrorType Cellular::getSignalStrength(DecibelMilliWatts &signalStrength) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::reset() {
    return ErrorType::NotImplemented;
}