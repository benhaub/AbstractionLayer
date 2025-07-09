//AbstractionLayer
#include "IpClientModule.hpp"
#include "NetworkAbstraction.hpp"
#include "OperatingSystemModule.hpp"

ErrorType IpClient::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &sock, const Milliseconds timeout) {
    sock = -1;
    bool doneConnecting = false;
    ErrorType callbackError = ErrorType::Failure;

    auto connectCb = [&](const Milliseconds timeout) -> ErrorType {
        disconnect();

        if (version != IpTypes::Version::IPv4) {
            PLT_LOGE(TAG, "only IPv4 is supported");
            callbackError = ErrorType::NotSupported;
            doneConnecting = true;
            _status.connected = false;
            return callbackError;
        }

        const _i16 protocolFamily = toSimpleLinkProtocolFamily(version, callbackError);
        if (ErrorType::Success == callbackError) {
            constexpr void * notUsingInterfaceContext = nullptr;
            uint16_t destinationIpListSize = 1;
            uint32_t destinationIp;
            SlNetSock_AddrIn_t localAddress;

            callbackError = fromPlatformError(SlNetIfWifi_getHostByName(notUsingInterfaceContext, const_cast<char *>(hostname.data()), hostname.length(), &destinationIp, &destinationIpListSize, protocolFamily));
            if (ErrorType::Success == callbackError) {
                const _i16 domain = toSimplelinkDomain(version, callbackError);

                if (ErrorType::Success == callbackError) {
                    const _i16 slProtocol = toSimpleLinkProtocol(protocol, callbackError);

                    if (ErrorType::Success == callbackError) {
                        const _i16 type = toSimpleLinkType(protocol, callbackError);

                        if (ErrorType::Success == callbackError) {
                            callbackError = ErrorType::Failure;
                            _socket = SlNetIfWifi_socket(nullptr, domain, type, slProtocol, nullptr);

                            if (_socket >= 0) {
                                localAddress.sin_family = protocolFamily;
                                localAddress.sin_addr.s_addr = SlNetUtil_htonl(destinationIp);
                                localAddress.sin_port = SlNetUtil_htons(port);
                                constexpr uint8_t notUsingFlags = 0;
                                constexpr void * notUsingSocketContext = nullptr;

                                const int32_t connectReturn = SlNetIfWifi_connect(_socket, notUsingSocketContext, reinterpret_cast<SlNetSock_Addr_t *>(&localAddress), sizeof(localAddress), notUsingFlags);
                                if (connectReturn >= 0) {
                                    callbackError = ErrorType::Success;
                                    sock = _socket;
                                }
                                else {
                                    PLT_LOGW(TAG, "Connection error");
                                    callbackError = ErrorType::Failure;
                                }
                            }
                        }
                    }
                }
            }
        }

        _status.connected = callbackError == ErrorType::Success;
        doneConnecting = true;
        return callbackError;
    };

    ErrorType error = ErrorType::Failure;
    EventQueue::Event event = EventQueue::Event(std::bind(connectCb, timeout));
    if (ErrorType::Success != (error = network().addEvent(event))) {
        return error;
    }

    while (!doneConnecting) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpClient::disconnect() {
    if (_socket != -1) {
        SlNetIfWifi_close(_socket, nullptr);
        _socket = -1;
    }

    return ErrorType::Success;
}

ErrorType IpClient::sendBlocking(const std::string &data, const Milliseconds timeout) {
    bool doneSending = false;
    ErrorType callbackError = ErrorType::Failure;

    auto tx = [&](const std::string &frame, const Milliseconds timeout) -> ErrorType {
        callbackError = network().txBlocking(frame, _socket, timeout);
        if (ErrorType::Success != callbackError) {
            _status.connected = false;
        }

        doneSending = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(tx, data, timeout));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!doneSending) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpClient::receiveBlocking(std::string &buffer, const Milliseconds timeout) {
    bool doneReceiving = false;
    ErrorType callbackError = ErrorType::Failure;

    auto rx = [&]() -> ErrorType {
        callbackError = network().rxBlocking(buffer, _socket, timeout);
        if (ErrorType::Success != callbackError) {
            _status.connected = false;
        }

        doneReceiving = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(rx));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!doneReceiving) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpClient::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    auto tx = [&, callback, data, timeout]() -> ErrorType {
        ErrorType error = ErrorType::Failure;

        assert(nullptr != callback);
        assert(nullptr != data.get());

        error = network().txBlocking(*data, _socket, timeout);
        callback(error, data->size());

        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(tx));
    return network().addEvent(event);
}

ErrorType IpClient::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    auto rx = [&, callback, buffer, timeout]() -> ErrorType {
        ErrorType error = ErrorType::Failure;

        assert(nullptr != callback);
        assert(nullptr != buffer.get());

        error = network().rxBlocking(*buffer, _socket, timeout);
        callback(error, buffer);

        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(rx));
    return network().addEvent(event);
}