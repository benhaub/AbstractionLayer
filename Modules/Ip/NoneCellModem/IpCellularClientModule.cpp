//AbstractionLayer
#include "IpCellularClientModule.hpp"
#include "CellularModule.hpp"

#define IP_CELLULAR_CLIENT_DEBUG 0

ErrorType IpCellularClient::connectTo(std::string_view hostname, const Port port, const IpClientTypes::Protocol protocol, const IpClientTypes::Version version, Socket &socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularClient::disconnect() {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularClient::sendBlocking(const std::string &data, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularClient::receiveBlocking(std::string &buffer, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularClient::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType IpCellularClient::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}