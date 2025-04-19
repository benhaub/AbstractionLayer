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

ErrorType Wifi::txBlocking(const std::string &frame, const Socket socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::txNonBlocking(const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::rxNonBlocking(std::shared_ptr<std::string> frameBuffer, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> frameBuffer)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::getSignalStrength(DecibelMilliWatts &signalStrength) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::mainLoop() {
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