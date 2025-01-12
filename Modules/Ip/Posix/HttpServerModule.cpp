//AbstractionLayer
#include "HttpServerModule.hpp"

ErrorType HttpServer::listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) {
    _ipServer->setNetwork(this->network());

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
    constexpr Bytes maxBufferSize = 512;
    std::string buffer(maxBufferSize, 0);
    ErrorType error = ErrorType::Failure;

    error = _ipServer->receiveBlocking(buffer, timeout, socket);

    return error;
}

ErrorType HttpServer::sendNonBlocking(const std::shared_ptr<HttpServerTypes::Response> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::receiveNonBlocking(std::shared_ptr<HttpServerTypes::Request> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}
