//AbstractionLayer
#include "IpClientModule.hpp"
#include "OperatingSystemModule.hpp"

ErrorType IpClient::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout) {
    bool doneConnecting = false;
    ErrorType callbackError = ErrorType::Failure;

    auto connectCb = [&](const Milliseconds timeout) -> ErrorType {
        disconnect();
        callbackError = network().connectTo(hostname, port, protocol, version, _socket, timeout);

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
    ErrorType error = network().disconnect(_socket);

    if (ErrorType::Success == error) {
        _socket = -1;
    }

    return error;
}

ErrorType IpClient::sendBlocking(const std::string &data, const Milliseconds timeout) {
    bool doneSending = false;
    ErrorType callbackError = ErrorType::Failure;

    auto tx = [&](const std::string &frame, const Milliseconds timeout) -> ErrorType {
        callbackError = network().transmit(frame, _socket, timeout);
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
        callbackError = network().receive(buffer, _socket, timeout);
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