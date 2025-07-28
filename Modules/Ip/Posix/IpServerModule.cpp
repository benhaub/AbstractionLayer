//AbstractionLayer
#include "IpServerModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Log.hpp"

ErrorType IpServer::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) {
    bool doneListening = false;
    ErrorType callbackError = ErrorType::Failure;
    _protocol = IpTypes::Protocol::Unknown;
    _version = IpTypes::Version::Unknown;
    _port = 0;
    _status.listening = false;

    network().closeConnection(_listenerSocket);

    auto listenCb = [&]() -> ErrorType {
        callbackError = network().listenTo(protocol, version, port, _listenerSocket);
        if (ErrorType::Success == callbackError) {
            _protocol = protocol;
            _version = version;
            _port = port;
            _status.listening = true;
        }

        _status.listening = callbackError == ErrorType::Success;
        doneListening = true;

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
        if (connectionsAcceptedIsAtMaximum()) {
            callbackError = ErrorType::LimitReached;
        }
        else {
            callbackError = network().acceptConnection(_listenerSocket, socket, timeout);
            if (ErrorType::Success == callbackError) {
                _connectedSockets.push_back(socket);
                _status.activeConnections = _connectedSockets.size();
            }
        }

        acceptConnectionDone = true;
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
        if (ErrorType::Success == network().closeConnection(socket)) {
            const auto closedSocket = std::find(_connectedSockets.begin(), _connectedSockets.end(), socket);
            if (_connectedSockets.end() != closedSocket) {
                _connectedSockets.erase(closedSocket);
                _status.activeConnections = _connectedSockets.size();
                callbackError = ErrorType::Success;
            }
            else {
                callbackError = ErrorType::NoData;
            }

        }
        else {
            callbackError = fromPlatformError(errno);
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