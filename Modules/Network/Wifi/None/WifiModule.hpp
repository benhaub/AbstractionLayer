#ifndef __WIFI_MODULE_HPP__
#define __WIFI_MODULE_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "WifiAbstraction.hpp"
//C++
#include <memory>
#include <string>

class Wifi final : public WifiAbstraction {

    public:
    Wifi() : WifiAbstraction() {}

    ErrorType init() override;
    ErrorType networkUp() override;
    ErrorType networkDown() override;
    ErrorType transmit(const std::string &frame, const Socket socket, const Milliseconds timeout) override;
    ErrorType receive(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) override;
    ErrorType getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) override;
    ErrorType getSignalStrength(DecibelMilliWatts &signalStrength) override;

    ErrorType radioOn() override;
    ErrorType radioOff() override;
    ErrorType setSsid(WifiTypes::Mode mode, const std::string &ssid) override;
    ErrorType setPassword(WifiTypes::Mode mode, const std::string &password) override;
    ErrorType setMode(WifiTypes::Mode mode) override;
    ErrorType setAuthMode(WifiTypes::AuthMode authMode) override;
};

#endif // __WIFI_MODULE_HPP__
