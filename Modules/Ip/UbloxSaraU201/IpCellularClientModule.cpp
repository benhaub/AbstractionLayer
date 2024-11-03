#include "IpCellularClientModule.hpp"

ErrorType IpClient::connectTo(const std::string hostname, const Port port, const IpClientSettings::Protocol protocol, const IpClientSettings::Version version, Socket &socket, const Milliseconds timeout) {
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