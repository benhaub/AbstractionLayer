#ifndef __CELLULAR_MODULE_HPP__
#define __CELLULAR_MODULE_HPP__

#include "CellularAbstraction.hpp"

class Cellular final : public CellularAbstraction {
    public:
    Cellular() : CellularAbstraction() {
        _status.isUp = false;
        _status.technology = NetworkTypes::Technology::Cellular;
    }

    ErrorType init() override;
    ErrorType networkUp() override;
    ErrorType networkDown() override;
    
    ErrorType transmit(const std::string &frame, const Socket socket, const Milliseconds timeout) override;
    ErrorType receive(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) override;
    ErrorType getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) override;
    ErrorType getSignalStrength(DecibelMilliWatts &signalStrength) override;

    ErrorType reset() override;
};

#endif // __CELLULAR_MODULE_HPP__