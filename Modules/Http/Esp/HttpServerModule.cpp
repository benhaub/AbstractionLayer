//AbstractionLayer
#include "HttpServerModule.hpp"

ErrorType HttpServer::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) {
    assert(nullptr != _network);
    return _ipServer.listenTo(protocol, version, port);
}

ErrorType HttpServer::acceptConnection(Socket &socket, const Milliseconds timeout) {
    assert(nullptr != _network);
    return _ipServer.acceptConnection(socket, timeout);
}

ErrorType HttpServer::closeConnection(const Socket socket) {
    assert(nullptr != _network);
    return _ipServer.closeConnection(socket);
}

ErrorType HttpServer::sendBlocking(const HttpTypes::Response &response, const Milliseconds timeout, const Socket socket) {
    assert(nullptr != _network);
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::receiveBlocking(HttpTypes::Request &request, const Milliseconds timeout, Socket &socket) {
    assert(nullptr != _network);
    const Bytes messageBodySize = request.messageBody.size();
    assert(messageBodySize > 0);
    ErrorType error = ErrorType::Failure;
    std::string &buffer = request.messageBody;

    if (0 == request.headers.contentLength) {
        do {
            request.messageBody.resize(messageBodySize);

            error = _ipServer.receiveBlocking(buffer, timeout, socket);

            if (ErrorType::Success == error) {
                error = readRequestHeaders(request, timeout);
            }
        } while (ErrorType::Negative == error);
    }

    return error;
}

ErrorType HttpServer::sendNonBlocking(const std::shared_ptr<HttpTypes::Response> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    //Big enough that hopefully the string doesn't have to reallocate.
    constexpr Bytes headerSize = 512;
    std::shared_ptr<std::string> frame = std::make_shared<std::string>(headerSize + data->messageBody.size(), 0);

    Bytes initialCapacity = frame->capacity();
    ToHttpResponse(*data, *frame);

    if (initialCapacity > frame->capacity()) {
        PLT_LOGW(TAG, "frame size had to be increased from %u to %u", initialCapacity, frame->capacity());
    }

    auto sendCallback = [callback](const ErrorType error, const Bytes bytesWritten) -> void {
        callback(error, bytesWritten);
    };

    return _ipServer.sendNonBlocking(frame, timeout, socket, sendCallback);
}

ErrorType HttpServer::receiveNonBlocking(std::shared_ptr<HttpTypes::Request> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<HttpTypes::Request> buffer)> callback) {
    constexpr Bytes maxBufferSize = 1448;
    std::shared_ptr<std::string> receivedBuffer = std::make_shared<std::string>(maxBufferSize, 0);

    auto receiveCallback = [&, callback](ErrorType error, const Socket socket, std::shared_ptr<std::string> frameBuffer) -> ErrorType {
        std::shared_ptr<HttpTypes::Request> request = std::make_shared<HttpTypes::Request>();
        ToHttpRequest(*frameBuffer, *request);
        callback(error, socket, request);

        return error;
    };

    //What if we don't receive the whole message?
    return _ipServer.receiveNonBlocking(receivedBuffer, timeout, receiveCallback);
}