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

ErrorType Wifi::getMacAddress(std::string &macAddress) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::getSignalStrength(DecibelMilliWatts &signalStrength) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::mainLoop() {
    return ErrorType::NotImplemented;
}

ErrorType radioOn() {
    return ErrorType::NotImplemented;
}

ErrorType radioOff() {
    return ErrorType::NotImplemented;
}

ErrorType setSsid(WifiConfig::Mode mode, const std::string &ssid) {
    return ErrorType::NotImplemented;
}

ErrorType setPassword(WifiConfig::Mode mode, const std::string &password) {
    return ErrorType::NotImplemented;
}

ErrorType setMode(WifiConfig::Mode mode) {
    return ErrorType::NotImplemented;
}

ErrorType setAuthMode(WifiConfig::AuthMode authMode) {
    return ErrorType::NotImplemented;
}