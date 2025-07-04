#include "IpClientModule.hpp"

ErrorType IpClient::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &sock, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType IpClient::disconnect() {
    return ErrorType::NotImplemented;
}
ErrorType IpClient::sendBlocking(const std::string &data, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType IpClient::receiveBlocking(std::string &buffer, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType IpClient::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType IpClient::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}