//AbstractionLayer
#include "HttpServerModule.hpp"

ErrorType HttpServer::listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::acceptConnection(Socket &socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::closeConnection(const Socket socket) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::sendBlocking(const HttpServerTypes::Response &response, const Milliseconds timeout, const Socket socket) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::receiveBlocking(HttpServerTypes::Request &request, const Milliseconds timeout, Socket &socket) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::sendNonBlocking(const std::shared_ptr<HttpServerTypes::Response> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType HttpServer::receiveNonBlocking(std::shared_ptr<HttpServerTypes::Request> buffer, const Milliseconds timeout, Socket &socket, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<HttpServerTypes::Request> buffer)> callback) {
    return ErrorType::NotImplemented;
}
