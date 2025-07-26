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