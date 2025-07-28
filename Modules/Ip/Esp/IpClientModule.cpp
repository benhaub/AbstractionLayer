//AbstractionLayer
#include "IpClientModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Log.hpp"
//Esp
//C++
#include <cassert>
#include <cstring>
#include <limits>

/*
 * I tried the example code for non-blocking sockets from ESP github, it didn't work.
 * getaddrinfo would return a new IP address every time you call it and none of them
 * worked for DNS. If you didn't call gethostbyname (which getaddrinfo is meant
 * to replace) getaddrinfo would fail to find an IP every time you call it. Lastly, if you
 * call fcntl and set O_NONBLOCK on the socket it would fail to connect every time so there
 * was quite a symphony of bugs there.
*/
ErrorType IpClient::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout) {
    bool doneConnecting = false;
    ErrorType callbackError = ErrorType::Failure;

    auto connectCb = [&]() -> ErrorType {
        disconnect();

        callbackError = network().connectTo(hostname, port, protocol, version, _socket, timeout);
        doneConnecting = true;
        _status.connected = callbackError == ErrorType::Success;

        return callbackError;

    };

    ErrorType error = ErrorType::Failure;
    EventQueue::Event event = EventQueue::Event(std::bind(connectCb));
    if (ErrorType::Success != (error = network().addEvent(event))) {
        return error;
    }

    while (!doneConnecting) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpClient::disconnect() {
    ErrorType error = ErrorType::Success;

    if (_socket != -1) {
        error = network().disconnect(_socket);
        _socket = -1;
    }

    return error;
}

ErrorType IpClient::sendBlocking(const std::string &data, const Milliseconds timeout) {
    assert(0 != _socket);
    bool doneSending = false;
    ErrorType callbackError = ErrorType::Failure;

    auto tx = [&]() -> ErrorType {
        callbackError = network().transmit(data, _socket, timeout);

        _status.connected = callbackError == ErrorType::Success;
        doneSending = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(tx));
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

        callbackError = network().receive(buffer, _socket, timeout);

        _status.connected = callbackError == ErrorType::Success;
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