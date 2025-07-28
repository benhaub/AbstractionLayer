#include "CellularModule.hpp"

ErrorType Cellular::init() {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::networkUp() {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::networkDown() {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &sock, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType Cellular::disconnect(const Socket &socket) {
    return ErrorType::NotImplemented;
}
ErrorType Cellular::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port, Socket &listenerSocket) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::acceptConnection(const Socket &listenerSocket, Socket &newSocket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::closeConnection(const Socket socket) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::transmit(const std::string &frame, const Socket socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::receive(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::getSignalStrength(DecibelMilliWatts &signalStrength) {
    return ErrorType::NotImplemented;
}

ErrorType Cellular::reset() {
    return ErrorType::NotImplemented;
}