//AbstractionLayer
#include "IpServer.hpp"

ErrorType IpServer::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) {
    bool doneListening = false;
    ErrorType callbackError = ErrorType::Failure;

    closeConnection(_listenerSocket);

    auto listenCb = [&]() -> ErrorType {
        callbackError = network().listenTo(protocol, version, port, _listenerSocket);
        if (ErrorType::Success == callbackError) {
            _protocol = protocol;
            _version = version;
            _port = port;
        }

        _status.listening = callbackError == ErrorType::Success;
        doneListening = true;

        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(listenCb);
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
        callbackError = network().acceptConnection(_listenerSocket, socket, timeout);

        if (ErrorType::Success == callbackError) {
            _connectedSockets.push_back(socket);
            _status.activeConnections = _connectedSockets.size();
        }

        acceptConnectionDone = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(acceptConnectionCallback);
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

    auto closeConnection = [&, socket]() -> ErrorType {
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

    EventQueue::Event event = EventQueue::Event(closeConnection);
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!closeConnectionDone) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpServer::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    auto tx = [&, callback, data, timeout, socket]() -> ErrorType {
        ErrorType error = ErrorType::Failure;

        assert(nullptr != callback);

        if (nullptr == data.get()) {
            error = ErrorType::NoData;
        }
        else {
            error = sendBlocking(*data, timeout, socket);
        }

        callback(error, data->size());
        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(tx));
    return network().addEvent(event);
}

ErrorType IpServer::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) {
    auto receiveCallback = [&, callback, buffer, timeout]() -> ErrorType {
        ErrorType error = ErrorType::Failure;
        Socket socket = -1;

        assert(nullptr != callback);

        if (nullptr == buffer.get()) {
            error = ErrorType::NoData;
            callback(error, socket, buffer);
            return error;
        }

        error = receiveBlocking(*buffer, timeout, socket);

        callback(error, socket, buffer);

        return error;
    };

    EventQueue::Event event = EventQueue::Event(receiveCallback);
    return network().addEvent(event);
}