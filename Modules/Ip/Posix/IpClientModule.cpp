//Modules
#include "IpClientModule.hpp"
#include "NetworkAbstraction.hpp"
#include "OperatingSystemModule.hpp"
//Posix
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
//C++
#include <cassert>
#include <cstring>

ErrorType IpClient::connectTo(const std::string &hostname, const Port port, const IpClientSettings::Protocol protocol, const IpClientSettings::Version version, Socket &sock, const Milliseconds timeout) {
    struct addrinfo hints;
    struct addrinfo *servinfo = nullptr;
    struct addrinfo *p = nullptr;
    char portString[] = "65535";

    //It's actually very important to run a memset on the hints struct before calling getaddrinfo.
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = toPosixSocktype(protocol);
    hints.ai_family = toPosixFamily(version);

    assert(snprintf(portString, sizeof(portString), "%u", port) > 0);

    int result = getaddrinfo(hostname.c_str(), portString, &hints, &servinfo);
    if (result != 0) {
        return toPlatformError(result);
    }

    for (p = servinfo; p != nullptr; p = p->ai_next) {
        if (-1 == (sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol))) {
            continue;
        }
        if (-1 == connect(sock, p->ai_addr, p->ai_addrlen)) {
            close(sock);
            sock = -1;
            return toPlatformError(errno);
        }

        break;
    }

    if (p == nullptr) {
        return toPlatformError(errno);
    }

    freeaddrinfo(servinfo);

    assert(-1 != sock);
    _socket = sock;
    _protocol = protocol;

    _status.connected = true;
    return ErrorType::Success;
}

ErrorType IpClient::disconnect() {
    return ErrorType::NotImplemented;
}

//TODO: Timeout is not implemented
ErrorType IpClient::sendBlocking(const std::string &data, const Milliseconds timeout) {
    assert(0 != _socket);

    if (-1 == send(_socket, data.data(), data.size(), 0)) {
        _status.connected = false;
        return toPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType IpClient::receiveBlocking(std::string &buffer, const Milliseconds timeout) {
    assert(0 != _socket);

    ErrorType error = ErrorType::Failure;
    ssize_t bytesReceived = 0;

    struct timeval timeoutval = {
        .tv_sec = timeout / 1000,
        .tv_usec = 0
    };
    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(_socket, &readfds);

    //Wait for input from the socket until the timeout
    {
    int ret;
    ret = select(_socket + 1, &readfds, NULL, NULL, &timeoutval);
    if (ret < 0) {
        return toPlatformError(errno);
    }
    }

    if (FD_ISSET(_socket, &readfds)) {
        if (-1 == (bytesReceived = recv(_socket, buffer.data(), buffer.size(), 0))) {
            error = toPlatformError(errno);
        }
        else if ((size_t)bytesReceived > buffer.size()) {
            error = ErrorType::PrerequisitesNotMet;
        }
        else {
            buffer.resize(bytesReceived);
            return ErrorType::Success;
        }
    }
    else {
        error = ErrorType::Timeout;
    }

    buffer.resize(0);
    _status.connected = false;

    return error;
}

ErrorType IpClient::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    bool sent = false;

    auto tx = [this, callback, &sent](const std::shared_ptr<std::string> frame, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        if (nullptr == frame.get()) {
            assert(false);
            return ErrorType::NoData;
        }

        error = sendBlocking(*frame, timeout);

        if (nullptr != callback) {
            callback(error, frame->size());
        }

        sent = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<IpClient>>(std::bind(tx, data, timeout));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    //Block for the timeout specified if no callback is provided
    if (nullptr == callback) {
        Milliseconds i;
        for (i = 0; i < timeout / 10 && !sent; i++) {
            OperatingSystem::Instance().delay(10);
        }

        if (!sent && (timeout / 10) == i) {
            return ErrorType::Timeout;
        }
        else if (!sent) {
            return ErrorType::Failure;
        }
        else {
            return ErrorType::Success;
        }
    }

    return ErrorType::Success;
}

ErrorType IpClient::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    bool received = false;

    auto rx = [this, callback, &received](const std::shared_ptr<std::string> buffer, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        if (nullptr == buffer.get()) {
            assert(false);
            return ErrorType::NoData;
        }

        error = receiveBlocking(*buffer, timeout);

        if (nullptr != callback) {
            callback(error, buffer);
        }

        received = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<IpClient>>(std::bind(rx, buffer, timeout));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    //Block for the timeout specified if no callback is provided
    if (nullptr == callback) {
        Milliseconds i;
        for (i = 0; i < timeout / 10 && !received; i++) {
            OperatingSystem::Instance().delay(10);
        }

        if (!received && (timeout / 10) == i) {
            return ErrorType::Timeout;
        }
        else if (!received) {
            return ErrorType::Failure;
        }
        else {
            return ErrorType::Success;
        }
    }

    return ErrorType::Success;
}
