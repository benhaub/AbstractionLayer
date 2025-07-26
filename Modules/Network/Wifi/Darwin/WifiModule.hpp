/***************************************************************************//**
* @author   Ben Haubrich
* @file     WifiModule.hpp
* @details  Wifi for posix compliant systems.
* @ingroup  PosixModules
*******************************************************************************/
#ifndef __WIFI_MODULE_HPP__
#define __WIFI_MODULE_HPP__

//AbstractionLayer
#include "WifiAbstraction.hpp"
#include "Error.hpp"
//C++
#include <memory>
#include <string>

class Wifi final : public WifiAbstraction {

    public:
    Wifi() : WifiAbstraction() {
        //Wifi on a posix system is always up.
        _status.isUp = true;
    }

    ErrorType init() override;
    ErrorType networkUp() override;
    ErrorType networkDown() override;
    ErrorType transmit(const std::string &frame, const Socket socket, const Milliseconds timeout) override;
    ErrorType receive(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) override;
    ErrorType getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) override;
    ErrorType getSignalStrength(DecibelMilliWatts &signalStrength) override;

    ErrorType radioOn() override { return ErrorType::NotAvailable; }
    ErrorType radioOff() override { return ErrorType::NotAvailable; }
    ErrorType setSsid(WifiTypes::Mode mode, const std::string &ssid) override { return ErrorType::NotAvailable; }
    ErrorType setPassword(WifiTypes::Mode mode, const std::string &password) override { return ErrorType::NotAvailable; }
    ErrorType setMode(WifiTypes::Mode mode) override { return ErrorType::NotAvailable; }
    ErrorType setAuthMode(WifiTypes::AuthMode authMode) override { return ErrorType::NotAvailable; }
};

#endif // __WIFI_MODULE_HPP__
