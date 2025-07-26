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