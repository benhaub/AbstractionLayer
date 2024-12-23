#ifndef __WIFI_MODULE_HPP__
#define __WIFI_MODULE_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "WifiAbstraction.hpp"
//C++
#include <memory>
#include <string>
//TI Drivers
#include "ti/drivers/net/wifi/device.h"

class Wifi : public WifiAbstraction {

    public:
    Wifi() : WifiAbstraction() {
        _status.isUp = false;
    }
    ~Wifi() = default;

    ErrorType init() override;
    ErrorType networkUp() override;
    ErrorType networkDown() override;
    ErrorType txBlocking(const std::string &frame, const Socket socket, const Milliseconds timeout) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> frameBuffer, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> frameBuffer)> callback) override;
    ErrorType getMacAddress(std::string &macAddress) override;
    ErrorType getSignalStrength(DecibelMilliWatts &signalStrength) override;

    ErrorType mainLoop() override;

    ErrorType radioOn() override;
    ErrorType radioOff() override;
    ErrorType setSsid(WifiConfig::Mode mode, std::string ssid) override;
    ErrorType setPassword(WifiConfig::Mode mode, std::string password) override;
    ErrorType setMode(WifiConfig::Mode mode) override;
    ErrorType setAuthMode(WifiConfig::AuthMode authMode) override;

    private:
    WifiConfig::Mode fromCc32xxRole(const _i16 role) {
        switch (role) {
            case ROLE_STA:
                return WifiConfig::Mode::Station;
            case ROLE_AP:
                return WifiConfig::Mode::AccessPoint;
            case ROLE_P2P:
                return WifiConfig::Mode::PointToPoint;
            case ROLE_TAG:
            default:
                return WifiConfig::Mode::Unknown;
        }
    }

    uint8_t toCc32xxRole(const WifiConfig::Mode mode, ErrorType &error) {
        error = ErrorType::Success;

        switch (mode) {
            case WifiConfig::Mode::Station:
                return ROLE_STA;
            case WifiConfig::Mode::AccessPoint:
                return ROLE_AP;
            case WifiConfig::Mode::PointToPoint:
                return ROLE_P2P;
            case WifiConfig::Mode::Unknown:
            default:
                error = ErrorType::InvalidParameter;
                return ROLE_STA;
        }
    }
};

#endif // __WIFI_MODULE_HPP__
