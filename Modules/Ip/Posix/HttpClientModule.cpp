//AbstractionLayer
#include "HttpClientModule.hpp"

ErrorType HttpClient::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &socket, const Milliseconds timeout) {
    assert(nullptr != _ipClient);
    socket = -1;

    return _ipClient->connectTo(hostname, port, protocol, version, socket, timeout);
}

ErrorType HttpClient::disconnect() {
    assert(nullptr != _ipClient);

    return _ipClient->disconnect();
}

ErrorType HttpClient::sendBlocking(const HttpTypes::Request &request, const Milliseconds timeout) {
    assert(nullptr != _ipClient);
    //Big enough that hopefully the string doesn't have to reallocate.
    constexpr Bytes headerSize = 512;
    std::string frame(headerSize + request.messageBody.size(), 0);

    const Bytes initialCapacity = frame.capacity();
    HttpTypes::fromHttpRequest(request, frame);

    if (initialCapacity > frame.capacity()) {
        PLT_LOGW(TAG, "frame size had to be increased from %u to %u", initialCapacity, frame.capacity());
    }

    return _ipClient->sendBlocking(frame, timeout);
}

ErrorType HttpClient::receiveBlocking(HttpTypes::Response &response, const Milliseconds timeout) {
    assert(nullptr != _ipClient);
    assert(response.messageBody.size() > 0);
    ErrorType error = ErrorType::Success;

    if (response.representationHeaders.contentLength == 0) {
            error = readResponseHeaders(response, timeout);
    }
    else {
        error = _ipClient->receiveBlocking(response.messageBody, timeout);
    }

    return error;
}

ErrorType HttpClient::sendNonBlocking(const std::shared_ptr<HttpTypes::Request> request, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    assert(nullptr != _ipClient);

    return ErrorType::NotImplemented;
}

ErrorType HttpClient::receiveNonBlocking(std::shared_ptr<HttpTypes::Response> request, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<HttpTypes::Request> buffer)> callback) {
    assert(nullptr != _ipClient);

    return ErrorType::NotImplemented;
}
