#ifndef __WIFI_MODULE_HPP__
#define __WIFI_MODULE_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "NetworkAbstraction.hpp"
//C++
#include <memory>
#include <string>

namespace WifiConfig {

    enum class Mode : uint8_t {
        Unknown = 0,
        AccessPoint, 
        Station,     
        AccessPointAndStation ///< Access Point and Station.
    };

    enum class AuthMode : uint8_t {
        Unknown = 0, 
        Open,        
        Wep,         
        WpaPsk,      
        Wpa2Psk      
    };
}

class Wifi : public NetworkAbstraction {

    public:
    Wifi() : NetworkAbstraction() {
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

    ErrorType setSsid(WifiConfig::Mode mode, std::string ssid) { return ErrorType::NotImplemented; }
    ErrorType setPassword(WifiConfig::Mode mode, std::string password) { return ErrorType::NotImplemented; }
    ErrorType setMode(WifiConfig::Mode mode) { return ErrorType::NotImplemented; }
    ErrorType setAuthMode(WifiConfig::AuthMode authMode) { return ErrorType::NotImplemented; }
};

#endif // __WIFI_MODULE_HPP__
