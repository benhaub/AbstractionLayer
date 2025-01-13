#include "IpServerModule.hpp"
#include "WifiModule.hpp"
#include "OperatingSystemModule.hpp"

//Posix
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
//Stdlib
#include <unistd.h>
//C++
#include <cassert>
#include <cstring>

ErrorType IpServer::listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) {
    Socket sock = -1;
    struct addrinfo hints;
    struct addrinfo *servinfo = nullptr;
    struct addrinfo *p = nullptr;
    char portString[] = "65535";

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = toPosixFamily(version);
    hints.ai_socktype = toPosixSocktype(protocol);
    hints.ai_flags = AI_PASSIVE;

    assert(snprintf(portString, sizeof(portString), "%u", port) > 0);

    if (0 != getaddrinfo(nullptr, portString, &hints, &servinfo)) {
        return fromPlatformError(errno);
    }

    for (p = servinfo; p != nullptr; p = p->ai_next) {
        if (-1 == (sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol))) {
            continue;
        }
        
        int enable = 1;
        if (-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable))) {
            return fromPlatformError(errno);
        }

        if (-1 == bind(sock, p->ai_addr, p->ai_addrlen)) {
            close(sock);
            continue;
        }

        break;
    }

    //For more connections, create another instance of this class.
    _status.listening = true;
    if (-1 == listen(sock, 1)) {
        _status.listening = false;
        return fromPlatformError(errno);
    }

    freeaddrinfo(servinfo);

    //Socket is still invalid. The socket we just had is only for listening for connections.
    //The socket we get from accept can be used to send and receive which is the one we want
    //to return to the user.
    _listenerSocket = sock;
    _protocol = protocol;
    _version = version;
    _port = port;

    return ErrorType::Success;
}
ErrorType IpServer::acceptConnection(Socket &socket, const Milliseconds timeout) {
    struct sockaddr_storage clientAddress;
    socklen_t receiveSocketSize;

    if (connectionsAcceptedIsAtMaximum()) {
        socket = -1;
        return ErrorType::LimitReached;
    }
    else {

        struct timeval timeoutval = {
            .tv_sec = timeout / 1000,
            .tv_usec = 0
        };
        fd_set readfds;

        FD_ZERO(&readfds);
        FD_SET(_listenerSocket, &readfds);

        {
        int ret;
        ret = select(_listenerSocket + 1, &readfds, NULL, NULL, &timeoutval);
        if (ret < 0) {
            return fromPlatformError(errno);
        }
        }

        if (FD_ISSET(_listenerSocket, &readfds)) {
            if (-1 == (socket = accept(_listenerSocket, (struct sockaddr *)&clientAddress, &receiveSocketSize))) {
                return fromPlatformError(errno);
            }
        }
        else {
            return ErrorType::Timeout;
        }
    }

    _connectedSockets.push_back(socket);
    _status.activeConnections = _connectedSockets.size();

    return ErrorType::Success;
}

ErrorType IpServer::closeConnection(const Socket socket) {
    if (-1 == close(socket)) {
        return fromPlatformError(errno);
    }

    const auto closedSocket = std::find(_connectedSockets.begin(), _connectedSockets.end(), socket);
    if (_connectedSockets.end() != closedSocket) {
        _connectedSockets.erase(closedSocket);
        _status.activeConnections = _connectedSockets.size();
        return ErrorType::Success;
    }

    return ErrorType::NoData;
}

ErrorType IpServer::sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket) {
    bool sent = false;
    ErrorType error = ErrorType::Failure;

    auto tx = [this, &error, &sent](const std::string &frame, const Socket socket, const Milliseconds timeout) -> ErrorType {
        error = network().txBlocking(frame, socket, timeout);

        sent = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<IpServer>>(std::bind(tx, data, socket, timeout));
    error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!sent) {
        OperatingSystem::Instance().delay(timeout);
    }

    return error;
}

ErrorType IpServer::receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket) {
    bool received = false;
    socket = -1;
    ErrorType error = ErrorType::Failure;

    auto rx = [this, &error, &received](std::string *buffer, Socket &socket, const Milliseconds timeout) -> ErrorType {
        //TODO: What if we only receive part of the data. We will need to keep this socket in play until we receive the whole thing.
        for (size_t i = _previousReceivedSocketIndex; i < _connectedSockets.size(); i++) {
            error = network().rxBlocking(*buffer, _connectedSockets[i], timeout);
            if (ErrorType::Success == error) {
                socket = _connectedSockets[i];
                _previousReceivedSocketIndex = i;
                break;
            }
        }

        received = true;
        return error;
    };

    //For some reason, the buffer can't be passed as a reference parameter to the callback. It has to be a pointer otherwise the
    //pointer to the data inside the string will change.
    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<IpServer>>(std::bind(rx, &buffer, socket, timeout));
    error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!received) {
        OperatingSystem::Instance().delay(timeout);
    }

    return error;
}

ErrorType IpServer::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    bool sent = false;

    auto tx = [this, callback, &sent](const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        if (nullptr == frame.get()) {
            return ErrorType::NoData;
        }

        error = network().txBlocking(*frame, socket, timeout);

        assert(nullptr != callback);
        callback(error, frame->size());

        sent = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<IpServer>>(std::bind(tx, data, socket, timeout));
    return network().addEvent(event);
}

ErrorType IpServer::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) {
    bool received = false;

    auto rx = [this, callback, &received](const std::shared_ptr<std::string> buffer, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        if (nullptr == buffer.get()) {
            return ErrorType::NoData;
        }

        //error = network().rxBlocking(*buffer, _socket, timeout);

        //assert(nullptr != callback);
        //callback(error, socket, buffer);

        received = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<IpServer>>(std::bind(rx, buffer, timeout));
    return network().addEvent(event);
}