//AbstractionLayer
#include "HttpServerModule.hpp"

ErrorType HttpServer::listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) {
    _ipServer->setNetwork(this->network());

    return _ipServer->listenTo(protocol, version, port);
}

ErrorType HttpServer::acceptConnection(Socket &socket) {
    return _ipServer->acceptConnection(socket);
}

ErrorType HttpServer::closeConnection() {
    return _ipServer->closeConnection();
}

ErrorType HttpServer::sendBlocking(const HttpServerTypes::Response &response, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::receiveBlocking(HttpServerTypes::Request &request, const Milliseconds timeout) {
    constexpr Bytes maxBufferSize = 512;
    std::string buffer(maxBufferSize, 0);
    ErrorType error = ErrorType::Failure;

    bool notCompletelyReceived = maxBufferSize == buffer.size();
    while (notCompletelyReceived) {
        ErrorType error = _ipServer->receiveBlocking(buffer, timeout);
        notCompletelyReceived = maxBufferSize == buffer.size();
        if (ErrorType::Timeout == error) {
            break;
        }
    }

    return error;
}

ErrorType HttpServer::sendNonBlocking(const std::shared_ptr<HttpServerTypes::Response> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::receiveNonBlocking(std::shared_ptr<HttpServerTypes::Request> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}
