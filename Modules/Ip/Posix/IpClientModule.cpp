//AbstractionLayer
#include "NetworkAbstraction.hpp"
#include "IpClientModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Log.hpp"
//Posix
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

//C++
#include <cassert>
#include <cstring>
#include <limits>

ErrorType IpClient::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &sock, const Milliseconds timeout) {
    sock = -1;
    bool doneConnecting = false;
    ErrorType callbackError = ErrorType::Failure;

    auto connectCb = [&](const Milliseconds timeout) -> ErrorType {
        // Ensure any existing connection is properly closed
        disconnect();
        signal(SIGPIPE, SIG_IGN);

        if (version == IpTypes::Version::IPv4) {
            struct addrinfo hints;
            struct addrinfo *res = nullptr;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = toPosixSocktype(protocol);
            
            std::array<char, sizeof(port) * 3> portString;
            snprintf(portString.data(), portString.size(), "%u", port);

            const int status = getaddrinfo(hostname.data(), portString.data(), &hints, &res);
            if (0 == status) {
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
                struct sockaddr_in dest_ip;
                memcpy(&dest_ip, ipv4, sizeof(dest_ip));
                dest_ip.sin_port = htons(port);

                if (-1 != (_socket = socket(toPosixFamily(version), toPosixSocktype(protocol), IPPROTO_IP))) {
                    if (0 == connect(_socket, (struct sockaddr *)&dest_ip, sizeof(dest_ip))) {
                        fd_set fdset;
                        FD_ZERO(&fdset);
                        FD_SET(_socket, &fdset);

                        Microseconds tvUsec = timeout * 1000;
                        struct timeval timeoutval;
                        if (tvUsec > std::numeric_limits<decltype(timeoutval.tv_usec)>::max()) {
                            PLT_LOGW(TAG, "Truncating microseconds because it is bigger than the type used by this platform.");
                            tvUsec = std::numeric_limits<decltype(timeoutval.tv_usec)>::max();
                        }
                        //There is some a limit on the amount of usec's that can be used for Darwin but the limit is not stated, so try to use seconds if possible
                        if (timeout >= 1000) {
                            timeoutval.tv_sec = timeout / 1000;
                            timeoutval.tv_usec = 0;
                        }
                        else {
                            timeoutval.tv_sec = 0;
                            timeoutval.tv_usec = static_cast<decltype(timeoutval.tv_usec)>(tvUsec);
                        }

                        // Connection in progress -> have to wait until the connecting socket is marked as writable, i.e. connection completes
                        int res = select(_socket+1, NULL, &fdset, NULL, &timeoutval);
                        if (res < 0) {
                            PLT_LOGW(TAG, "Error during connection: select for socket to be writable %s", strerror(errno));
                            callbackError = ErrorType::Failure;
                        }
                        else if (res == 0) {
                            PLT_LOGW(TAG, "Connection timeout: select for socket to be writable %s", strerror(errno));
                            callbackError = ErrorType::Timeout;
                        }
                        else {
                            int sockerr;
                            socklen_t len = (socklen_t)sizeof(int);

                            if (getsockopt(_socket, SOL_SOCKET, SO_ERROR, (void*)(&sockerr), &len) < 0) {
                                PLT_LOGW(TAG, "Error when getting socket error using getsockopt() %s", strerror(errno));
                                callbackError = ErrorType::Failure;
                            }
                            else if (sockerr) {
                                PLT_LOGW(TAG, "Connection error %d", sockerr);
                                callbackError = ErrorType::Failure;
                            }
                            else {
                                sock = _socket;
                                callbackError = ErrorType::Success;
                            }
                        }
                    }
                    else {
                        PLT_LOGW(TAG, "Failed to connect: %s", strerror(errno));
                        callbackError = fromPlatformError(errno);
                    }
                }
                else {
                    PLT_LOGW(TAG, "Failed to create socket: %s", strerror(errno));
                    callbackError = fromPlatformError(errno);
                }
            }
            else {
                PLT_LOGW(TAG, "Failed to get address info: %s", gai_strerror(status));
                callbackError = ErrorType::PrerequisitesNotMet;
            }

            freeaddrinfo(res);
        }
        else {
            callbackError = ErrorType::NotSupported;
        }

        doneConnecting = true;
        _status.connected = callbackError == ErrorType::Success;
        return callbackError;
    };

    ErrorType error = ErrorType::Failure;
    EventQueue::Event event = EventQueue::Event(std::bind(connectCb, timeout));
    if (ErrorType::Success != (error = network().addEvent(event))) {
        PLT_LOGW(TAG, "Could not add connection event to network");
        return error;
    }

    while (!doneConnecting) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpClient::disconnect() {
    if (_socket != -1) {
        shutdown(_socket, 0);
        close(_socket);
        _socket = -1;
    }

    return ErrorType::Success;
}

ErrorType IpClient::sendBlocking(const std::string &data, const Milliseconds timeout) {
    assert(0 != _socket);
    bool doneSending = false;
    ErrorType callbackError = ErrorType::Failure;

    auto tx = [&](const std::string &frame, const Milliseconds timeout) -> ErrorType {
        callbackError = network().txBlocking(frame, _socket, timeout);

        doneSending = true;
        _status.connected = callbackError == ErrorType::Success;
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
        assert(0 != _socket);

        callbackError = network().rxBlocking(buffer, _socket, timeout);

        doneReceiving = true;
        _status.connected = callbackError == ErrorType::Success;
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
    assert(0 != _socket);

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
    assert(0 != _socket);

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