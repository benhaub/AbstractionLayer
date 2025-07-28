//AbstractionLayer
#include "IpServerModule.hpp"
#include "OperatingSystemModule.hpp"

ErrorType IpServer::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) {
    bool listeningDone = false;
    ErrorType callbackError = ErrorType::Failure;

    auto listenCallback = [&]() -> ErrorType {
        callbackError = network().listenTo(protocol, version, port, _listenerSocket);
        if (ErrorType::Success == callbackError) {
            //Socket is still invalid. The socket we just had is only for listening for connections.
            //The socket we get from accept can be used to send and receive which is the one we want
            //to return to the user.
            _protocol = protocol;
            _version = version;
            _port = port;
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
        callbackError = network().closeConnection(socket);

        if (ErrorType::Success == callbackError) {
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