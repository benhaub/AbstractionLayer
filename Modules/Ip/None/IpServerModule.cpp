#include "IpServerModule.hpp"

ErrorType IpServer::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) {
    return ErrorType::NotImplemented;
}

ErrorType IpServer::acceptConnection(Socket &socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType IpServer::closeConnection(const Socket socket) {
    return ErrorType::NotImplemented;
}

ErrorType IpServer::sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket) {
    return ErrorType::NotImplemented;
}

ErrorType IpServer::receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket) {
    return ErrorType::NotImplemented;
}