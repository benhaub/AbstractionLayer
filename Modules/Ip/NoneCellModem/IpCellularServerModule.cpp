#include "IpCellularServerModule.hpp"

ErrorType IpCellularServer::listenTo(const IpServerTypes::Protocol protocol, const IpServerTypes::Version version, const Port port) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularServer::acceptConnection(Socket &socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularServer::closeConnection(const Socket socket) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularServer::sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularServer::receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularServer::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularServer::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, Socket &socket, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}