//AbstractionLayer
#include "IpServerModule.hpp"
#include "OperatingSystemModule.hpp"
#include "NetworkAbstraction.hpp"
#include "Log.hpp"
//C++
#include <limits>

ErrorType IpServer::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) {
    bool listeningDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto listenCallback = [&]() -> ErrorType {
        struct sockaddr_storage destinationAddress;
        int ipProtocol;

        if (IpTypes::Version::IPv4 == version) {
            struct sockaddr_in *ipv4DestinationAddress = reinterpret_cast<struct sockaddr_in *>(&destinationAddress);
            ipv4DestinationAddress->sin_addr.s_addr = htonl(INADDR_ANY);
            ipv4DestinationAddress->sin_family = AF_INET;
            ipv4DestinationAddress->sin_port = htons(port);
            ipProtocol = IPPROTO_IP;

            _listenerSocket = socket(AF_INET, toPosixSocktype(protocol), ipProtocol);
            if (_listenerSocket < 0) {
                listeningDone = true;
                callbackError = fromPlatformError(errno);
                return callbackError;
            }
        }
        else if (IpTypes::Version::IPv6 == version) {
            struct sockaddr_in6 *ipv6DestinationAddress = reinterpret_cast<struct sockaddr_in6 *>(&destinationAddress);
            bzero(&ipv6DestinationAddress->sin6_addr, sizeof(ipv6DestinationAddress->sin6_addr));
            ipv6DestinationAddress->sin6_family = AF_INET6;
            ipv6DestinationAddress->sin6_port = htons(port);
            ipProtocol = IPPROTO_IPV6;

            _listenerSocket = socket(AF_INET6, toPosixSocktype(protocol), ipProtocol);
            if (_listenerSocket < 0) {
                listeningDone = true;
                callbackError = fromPlatformError(errno);
                return callbackError;
            }
        }

        int enable = 1;
        if (-1 == setsockopt(_listenerSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable))) {
            listeningDone = true;
            callbackError = fromPlatformError(errno);
            return callbackError;
        }

        if (-1 == bind(_listenerSocket, reinterpret_cast<struct sockaddr *>(&destinationAddress), sizeof(destinationAddress))) {
            listeningDone = true;
            callbackError = fromPlatformError(errno);
            return callbackError;
        }

        if (0 == listen(_listenerSocket, 1)) {
            //Socket is still invalid. The socket we just had is only for listening for connections.
            //The socket we get from accept can be used to send and receive which is the one we want
            //to return to the user.
            _protocol = protocol;
            _version = version;
            _port = port;
            callbackError = ErrorType::Success;
        }
        else {
            callbackError = fromPlatformError(errno);
        }

        listeningDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(listenCallback);
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!listeningDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpServer::acceptConnection(Socket &socket, const Milliseconds timeout) {
    bool acceptConnectionDone = false;
    ErrorType callbackError = ErrorType::Failure;
    socket = -1;

    auto acceptCallback = [&]() -> ErrorType {
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

    EventQueue::Event event = EventQueue::Event(acceptCallback);
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
        callbackError = network().txBlocking(data, socket, timeout);

        sent = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(tx);
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
                callbackError = network().rxBlocking(buffer, _connectedSockets[i], timeout);
                socket = _connectedSockets[i];
                if (ErrorType::Success == callbackError) {
                    break;
                }
            }
        }
        else {
            callbackError = network().rxBlocking(buffer, socket, timeout);
        }

        received = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(rx);
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!received) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpServer::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    auto tx = [&, callback](const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        if (nullptr == frame.get()) {
            error = ErrorType::NoData;
        }
        else {
            error = network().txBlocking(*frame, socket, timeout);
        }

        assert(nullptr != callback);
        callback(error, frame->size());
        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(tx, data, socket, timeout));
    return network().addEvent(event);
}

ErrorType IpServer::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, Socket &socket, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) {
    auto receiveCallback = [&, callback]() -> ErrorType {
        ErrorType error = ErrorType::Failure;

        if (nullptr == buffer.get()) {
            error = ErrorType::NoData;
            callback(error, socket, buffer);
            return error;
        }

        error = receiveBlocking(*buffer, timeout, socket);

        assert(nullptr != callback);
        callback(error, socket, buffer);

        return error;
    };

    EventQueue::Event event = EventQueue::Event(receiveCallback);
    return network().addEvent(event);
}