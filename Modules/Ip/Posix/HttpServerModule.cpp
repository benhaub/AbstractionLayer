//AbstractionLayer
#include "HttpServerModule.hpp"
#include "Log.hpp"

ErrorType HttpServer::listenTo(const IpServerTypes::Protocol protocol, const IpServerTypes::Version version, const Port port) {
    return _ipServer->listenTo(protocol, version, port);
}

ErrorType HttpServer::acceptConnection(Socket &socket, const Milliseconds timeout) {
    return _ipServer->acceptConnection(socket, timeout);
}

ErrorType HttpServer::closeConnection(const Socket socket) {
    return _ipServer->closeConnection(socket);
}

ErrorType HttpServer::sendBlocking(const HttpServerTypes::Response &response, const Milliseconds timeout, const Socket socket) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::receiveBlocking(HttpServerTypes::Request &request, const Milliseconds timeout, Socket &socket) {
    constexpr Bytes maxBufferSize = 1448;
    std::string buffer(maxBufferSize, 0);
    ErrorType error = ErrorType::Failure;

    error = _ipServer->receiveBlocking(buffer, timeout, socket);

    if (ErrorType::Success == error && buffer.size() > 0) {
        if (ErrorType::Success == toHttpRequest(buffer, request)) {
            while (request.messageBody.size() < request.headers.contentLength) {
                buffer.resize(std::min(maxBufferSize, request.headers.contentLength - static_cast<Bytes>(request.messageBody.size())), 0);
                error = _ipServer->receiveBlocking(buffer, timeout, socket);
                if (ErrorType::Success == error && buffer.size() > 0) {
                    request.messageBody.append(buffer);
                }
            }
        }
    }

    return error;
}

ErrorType HttpServer::sendNonBlocking(const std::shared_ptr<HttpServerTypes::Response> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    //Big enough that hopefully the string doesn't have to reallocate.
    constexpr Bytes headerSize = 512;
    std::shared_ptr<std::string> frame = std::make_shared<std::string>(headerSize + data->messageBody.size(), 0);

    const Bytes initialCapacity = frame->capacity();
    toHttpResponse(*data, *frame);

    if (initialCapacity > frame->capacity()) {
        PLT_LOGW(TAG, "frame size had to be increased from %u to %u", initialCapacity, frame->capacity());
    }

    //I believe if the callback is not taken by value it can go out of scope and cause a crash.
    auto sendCallback = [callback](const ErrorType error, const Bytes bytesWritten) -> void {
        callback(error, bytesWritten);
    };

    return _ipServer->sendNonBlocking(frame, timeout, socket, sendCallback);
}

ErrorType HttpServer::receiveNonBlocking(std::shared_ptr<HttpServerTypes::Request> buffer, const Milliseconds timeout, Socket &socket, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<HttpServerTypes::Request> buffer)> callback) {
    constexpr Bytes maxBufferSize = 1448;
    std::shared_ptr<std::string> receivedBuffer = std::make_shared<std::string>(maxBufferSize, 0);

    auto receiveCallback = [&, callback](ErrorType error, const Socket socket, std::shared_ptr<std::string> frameBuffer) -> ErrorType {
        std::shared_ptr<HttpServerTypes::Request> request = std::make_shared<HttpServerTypes::Request>();
        toHttpRequest(*frameBuffer, *request);
        callback(error, socket, request);

        return error;
    };

    //What if we don't receive the whole message?
    return _ipServer->receiveNonBlocking(receivedBuffer, timeout, socket, receiveCallback);
}