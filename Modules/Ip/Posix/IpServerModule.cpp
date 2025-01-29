//AbstractionLayer
#include "IpServerModule.hpp"
#include "OperatingSystemModule.hpp"
#include "NetworkAbstraction.hpp"

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
    Socket sock = _listenerSocket = -1;
    bool doneListening = false;
    ErrorType error = ErrorType::Failure;
    _protocol = IpServerSettings::Protocol::Unknown;
    _version = IpServerSettings::Version::Unknown;
    _port = 0;

    close(_listenerSocket);

    auto listenCb = [&]() -> ErrorType {
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
            error = fromPlatformError(errno);
            doneListening = true;
            return error;
        }

        for (p = servinfo; p != nullptr; p = p->ai_next) {
            if (-1 == (sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol))) {
                continue;
            }
            
            int enable = 1;
            if (-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable))) {
                error = fromPlatformError(errno);
                doneListening = true;
                return error;
            }

            if (-1 == bind(sock, p->ai_addr, p->ai_addrlen)) {
                close(sock);
                continue;
            }

            break;
        }

        //For more connections, create another instance of this class.
        if (0 == listen(sock, 1)) {
            //Socket is still invalid. The socket we just had is only for listening for connections.
            //The socket we get from accept can be used to send and receive which is the one we want
            //to return to the user.
            _listenerSocket = sock;
            _protocol = protocol;
            _version = version;
            _port = port;
            error = ErrorType::Success;
        }
        else {
            error = fromPlatformError(errno);
        }

        ErrorType::Success == error ? _status.listening = true : _status.listening = false;
        doneListening = true;
        freeaddrinfo(servinfo);

        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(listenCb));
    if (ErrorType::Success != network().addEvent(event)) {
        return ErrorType::Failure;
    }

    while (!doneListening) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType IpServer::acceptConnection(Socket &socket, const Milliseconds timeout) {
    bool acceptConnectionDone = false;
    ErrorType error = ErrorType::Failure;
    socket = -1;

    auto acceptConnectionCallback = [&]() -> ErrorType {
        struct sockaddr_storage clientAddress;
        socklen_t receiveSocketSize;

        if (connectionsAcceptedIsAtMaximum()) {
            acceptConnectionDone = true;
            error = ErrorType::LimitReached;
            return error;
        }
        else {
            struct timeval timeoutval = {
                .tv_sec = 0,
                .tv_usec = timeout * 1000
            };
            fd_set readfds;

            FD_ZERO(&readfds);
            FD_SET(_listenerSocket, &readfds);

            {
            int ret;
            ret = select(_listenerSocket + 1, &readfds, NULL, NULL, &timeoutval);
            if (ret < 0) {
                acceptConnectionDone = true;
                error = fromPlatformError(errno);
                return error;
            }
            }

            if (FD_ISSET(_listenerSocket, &readfds)) {
                if (-1 == (socket = accept(_listenerSocket, (struct sockaddr *)&clientAddress, &receiveSocketSize))) {
                    acceptConnectionDone = true;
                    assert(false);
                    error = fromPlatformError(errno);
                    return error;
                }
            }
            else {
                acceptConnectionDone = true;
                error = ErrorType::Timeout;
                return error;
            }
        }

        _connectedSockets.push_back(socket);
        _status.activeConnections = _connectedSockets.size();

        acceptConnectionDone = true;
        error = ErrorType::Success;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(acceptConnectionCallback));
    error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!acceptConnectionDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType IpServer::closeConnection(const Socket socket) {
    bool closeConnectionDone = false;
    ErrorType error = ErrorType::Failure;

    auto closeConnection = [this, &error, &closeConnectionDone](const Socket socket) -> ErrorType {
        if (-1 == close(socket)) {
            closeConnectionDone = true;
            error = fromPlatformError(errno);
            return error;
        }

        const auto closedSocket = std::find(_connectedSockets.begin(), _connectedSockets.end(), socket);
        if (_connectedSockets.end() != closedSocket) {
            _connectedSockets.erase(closedSocket);
            _status.activeConnections = _connectedSockets.size();
            error = ErrorType::Success;
        }
        else {
            error = ErrorType::NoData;
        }

        closeConnectionDone = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(closeConnection, socket));
    error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!closeConnectionDone) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType IpServer::sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket) {
    bool sent = false;
    ErrorType error = ErrorType::Failure;

    auto tx = [&]() -> ErrorType {
        error = network().txBlocking(data, socket, timeout);

        sent = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(tx));
    error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!sent) {
        OperatingSystem::Instance().delay(1);
    }

    return error;
}

ErrorType IpServer::receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket) {
    bool received = false;
    socket = -1;
    ErrorType error = ErrorType::Failure;

    auto rx = [&]() -> ErrorType {
        //TODO: What if we only receive part of the data. We will need to keep this socket in play until we receive the whole thing.
        for (size_t i = _previousReceivedSocketIndex; i < _connectedSockets.size(); i++) {
            error = network().rxBlocking(buffer, _connectedSockets[i], timeout);
            if (ErrorType::Success == error) {
                socket = _connectedSockets[i];
                _previousReceivedSocketIndex = i;
                break;
            }
        }

        received = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(rx));
    error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!received) {
        OperatingSystem::Instance().delay(1);
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

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(tx, data, socket, timeout));
    return network().addEvent(event);
}

ErrorType IpServer::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, Socket &socket, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) {
    bool received = false;

    auto receiveCallback = [&, callback]() -> ErrorType {
        ErrorType error = ErrorType::Failure;

        if (nullptr == buffer.get()) {
            error = ErrorType::NoData;
        }

        error = receiveBlocking(*buffer, timeout, socket);

        assert(nullptr != callback);
        callback(error, socket, buffer);

        received = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<>>(std::bind(receiveCallback));
    return network().addEvent(event);
}