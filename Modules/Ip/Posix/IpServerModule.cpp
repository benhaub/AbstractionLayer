//AbstractionLayer
#include "IpServerModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Log.hpp"
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
#include <limits>

ErrorType IpServer::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) {
    Socket sock = _listenerSocket = -1;
    bool doneListening = false;
    ErrorType callbackError = ErrorType::Failure;
    _protocol = IpTypes::Protocol::Unknown;
    _version = IpTypes::Version::Unknown;
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

        if (0 == getaddrinfo(nullptr, portString, &hints, &servinfo)) {
            for (p = servinfo; p != nullptr; p = p->ai_next) {
                if (-1 != (sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol))) {
                    int enable = 1;
                    if (-1 != setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable))) {
                        if (-1 != bind(sock, p->ai_addr, p->ai_addrlen)) {
                            //For more connections, create another instance of this class.
                            if (0 == listen(sock, 1)) {
                                //Socket is still invalid. The socket we just had is only for listening for connections.
                                //The socket we get from accept can be used to send and receive which is the one we want
                                //to return to the user.
                                _listenerSocket = sock;
                                _protocol = protocol;
                                _version = version;
                                _port = port;
                                callbackError = ErrorType::Success;
                                doneListening = true;
                                _status.listening = true;
                                return callbackError;
                            }
                        }
                    }

                    close(sock);
                    callbackError = fromPlatformError(errno);
                    continue;
                }
            }

            freeaddrinfo(servinfo);
        }

        _status.listening = false;
        doneListening = true;
        callbackError = fromPlatformError(errno);
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(listenCb));
    if (ErrorType::Success != network().addEvent(event)) {
        return ErrorType::Failure;
    }

    while (!doneListening) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpServer::acceptConnection(Socket &socket, const Milliseconds timeout) {
    bool acceptConnectionDone = false;
    ErrorType callbackError = ErrorType::Failure;
    socket = -1;

    auto acceptConnectionCallback = [&]() -> ErrorType {
        struct sockaddr_storage clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);

        if (connectionsAcceptedIsAtMaximum()) {
            acceptConnectionDone = true;
            callbackError = ErrorType::LimitReached;
            return callbackError;
        }
        else {
            Microseconds tvUsec = timeout * 1000;
            struct timeval timeoutval;
            if (tvUsec > std::numeric_limits<decltype(timeoutval.tv_usec)>::max()) {
                PLT_LOGW(TAG, "Truncating microseconds because it is bigger than the type used by this platform.");
                tvUsec = std::numeric_limits<decltype(timeoutval.tv_usec)>::max();
            }
            timeoutval.tv_sec = 0;
            timeoutval.tv_usec = static_cast<decltype(timeoutval.tv_usec)>(tvUsec);

            fd_set readfds;

            FD_ZERO(&readfds);
            FD_SET(_listenerSocket, &readfds);

            {
            int ret;
            ret = select(_listenerSocket + 1, &readfds, NULL, NULL, &timeoutval);
            if (ret < 0) {
                acceptConnectionDone = true;
                callbackError = fromPlatformError(errno);
                return callbackError;
            }
            }

            if (FD_ISSET(_listenerSocket, &readfds)) {
                if (-1 == (socket = accept(_listenerSocket, (struct sockaddr *)&clientAddress, &clientAddressSize))) {
                    acceptConnectionDone = true;
                    callbackError = fromPlatformError(errno);
                    return callbackError;
                }
            }
            else {
                acceptConnectionDone = true;
                callbackError = ErrorType::Timeout;
                return callbackError;
            }
        }

        _connectedSockets.push_back(socket);
        _status.activeConnections = _connectedSockets.size();

        acceptConnectionDone = true;
        callbackError = ErrorType::Success;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(acceptConnectionCallback));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!acceptConnectionDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpServer::closeConnection(const Socket socket) {
    bool closeConnectionDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto closeConnection = [&](const Socket socket) -> ErrorType {
        if (-1 == close(socket)) {
            closeConnectionDone = true;
            callbackError = fromPlatformError(errno);
            return callbackError;
        }

        const auto closedSocket = std::find(_connectedSockets.begin(), _connectedSockets.end(), socket);
        if (_connectedSockets.end() != closedSocket) {
            _connectedSockets.erase(closedSocket);
            _status.activeConnections = _connectedSockets.size();
            callbackError = ErrorType::Success;
        }
        else {
            callbackError = ErrorType::NoData;
        }

        closeConnectionDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(closeConnection, socket));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!closeConnectionDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpServer::sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket) {
    bool sent = false;
    ErrorType callbackError = ErrorType::Failure;

    auto tx = [&]() -> ErrorType {
        callbackError = network().transmit(data, socket, timeout);

        sent = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(tx));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!sent) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpServer::receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket) {
    bool received = false;
    ErrorType callbackError = ErrorType::NoData;

    auto rx = [&]() -> ErrorType {
        if (-1 == socket) {
            for (size_t i = 0; i < _connectedSockets.size(); i++) {
                callbackError = network().receive(buffer, _connectedSockets[i], timeout);
                socket = _connectedSockets[i];
                if (ErrorType::Success == callbackError) {
                    break;
                }
            }
        }
        else {
            callbackError = network().receive(buffer, socket, timeout);
        }

        received = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(rx));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!received) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}