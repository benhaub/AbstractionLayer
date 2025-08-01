#include "WifiModule.hpp"

ErrorType Wifi::init() {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::networkUp() {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::networkDown() {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &sock, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::disconnect(const Socket &socket) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port, Socket &listenerSocket) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::acceptConnection(const Socket &listenerSocket, Socket &newSocket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::closeConnection(const Socket socket) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::transmit(const std::string &frame, const Socket socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::receive(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::getSignalStrength(DecibelMilliWatts &signalStrength) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::radioOn() {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::radioOff() {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::setSsid(WifiTypes::Mode mode, const std::string &ssid) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::setPassword(WifiTypes::Mode mode, const std::string &password) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::setMode(WifiTypes::Mode mode) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::setAuthMode(WifiTypes::AuthMode authMode) {
    return ErrorType::NotImplemented;
}