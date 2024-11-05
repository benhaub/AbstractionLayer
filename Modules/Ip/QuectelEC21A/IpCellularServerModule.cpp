#include "IpCellularServerModule.hpp"

ErrorType IpCellularServer::listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularServer::acceptConnection(Socket &socket) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularServer::closeConnection() {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularServer::sendBlocking(const std::string &data, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularServer::receiveBlocking(std::string &buffer, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularServer::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularServer::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}