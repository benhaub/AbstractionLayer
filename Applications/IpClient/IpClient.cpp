//AbstractionLayer
#include "IpClient.hpp"

ErrorType IpClient::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout) {
    bool doneConnecting = false;
    ErrorType callbackError = ErrorType::Failure;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto connectCb = [&, thread]() -> ErrorType {
        // Ensure any existing connection is properly closed
        disconnect();

        callbackError = network().connectTo(hostname, port, protocol, version, _socket, timeout);

        doneConnecting = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    ErrorType error = ErrorType::Failure;
    EventQueue::Event event = EventQueue::Event(connectCb);
    if (ErrorType::Success != (error = network().addEvent(event))) {
        PLT_LOGW(TAG, "Could not add connection event to network");
        return error;
    }

    while (!doneConnecting) {
        OperatingSystem::Instance().block();
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

ErrorType IpClient::sendNonBlocking(const std::string &data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    auto tx = [&, callback, data = std::move(data), timeout]() -> ErrorType {
        ErrorType error = ErrorType::Failure;

        assert(nullptr != callback);

        error = sendBlocking(std::string_view(data.data(), data.size()), timeout);
        callback(error, data.size());

        return error;
    };

    EventQueue::Event event = EventQueue::Event(tx);
    return network().addEvent(event);
}

ErrorType IpClient::receiveNonBlocking(std::string &buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::string_view buffer)> callback) {
    auto rx = [&, callback, buffer = std::move(buffer), timeout]() mutable -> ErrorType {
        ErrorType error = ErrorType::Failure;

        assert(nullptr != callback);

        error = receiveBlocking(buffer, timeout);
        callback(error, std::string_view(buffer.data(), buffer.size()));

        return error;
    };

    EventQueue::Event event = EventQueue::Event(rx);
    return network().addEvent(event);
}