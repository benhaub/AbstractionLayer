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
ErrorType Cellular::txBlocking(const std::string &frame, const Socket socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType Cellular::txNonBlocking(const std::shared_ptr<std::string> frame, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType Cellular::rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) {
    return ErrorType::NotImplemented;
}
ErrorType Cellular::rxNonBlocking(std::shared_ptr<std::string> frameBuffer, const Socket socket, std::function<void(const ErrorType error, std::shared_ptr<std::string> frameBuffer)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType Cellular::getMacAddress(std::string &macAddress) {
    return ErrorType::NotImplemented;
}
ErrorType Cellular::getSignalStrength(DecibelMilliWatts &signalStrength) {
    return ErrorType::NotImplemented;
}
ErrorType Cellular::mainLoop() {
    return ErrorType::NotImplemented;
}