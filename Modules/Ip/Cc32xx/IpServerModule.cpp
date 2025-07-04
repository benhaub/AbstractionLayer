//AbstractionLayer
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

ErrorType IpServer::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType IpServer::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, Socket &socket, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}

#ifdef __cplusplus
extern "C" {

void SimpleLinkNetAppRequestMemFreeEventHandler(uint8_t *buffer) {
    return;
}

#endif

#ifdef __cplusplus
}
#endif