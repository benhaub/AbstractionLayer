/***************************************************************************//**
* @author   Ben Haubrich
* @file     WifiModule.hpp
* @details  Wifi for ESP32
* @ingroup  EspModules
*******************************************************************************/
#ifndef __WIFI_MODULE_HPP__
#define __WIFI_MODULE_HPP__

//AbstractionLayer
#include "WifiAbstraction.hpp"
//ESP
#include "esp_wifi.h"

/**
 * @class Wifi
 * @brief Wifi for ESP32
*/
class Wifi : public WifiAbstraction {

    public:
    Wifi() : WifiAbstraction() {}
    ~Wifi() = default;

    ErrorType init() override;
    ErrorType networkUp() override;
    ErrorType networkDown() override;
    ErrorType txBlocking(const std::string &frame, const Socket socket, const Milliseconds timeout) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> frameBuffer, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> frameBuffer)> callback) override;
    ErrorType getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) override;
    ErrorType getSignalStrength(DecibelMilliWatts &signalStrength) override;

    ErrorType mainLoop() override;

    ErrorType radioOn() override;
    ErrorType radioOff() override;
    ErrorType setSsid(WifiConfig::Mode mode, const std::string &ssid) override;
    ErrorType setPassword(WifiConfig::Mode mode, const std::string &password) override;
    ErrorType setMode(WifiConfig::Mode mode) override { _mode = mode; return ErrorType::Success; }
    ErrorType setAuthMode(WifiConfig::AuthMode authMode) override { _authMode = authMode; return ErrorType::Success; }

    private:
    wifi_auth_mode_t toEspAuthMode(WifiConfig::AuthMode authMode) {
        switch (authMode) {
            case WifiConfig::AuthMode::Open:
                return WIFI_AUTH_OPEN;
            case WifiConfig::AuthMode::Wep:
                return WIFI_AUTH_WEP;
            case WifiConfig::AuthMode::Wpa:
                return WIFI_AUTH_WPA_PSK;
            case WifiConfig::AuthMode::WpaWpa2:
                return WIFI_AUTH_WPA2_PSK;
            default:
                return WIFI_AUTH_OPEN;
        }
    }

    wifi_mode_t toEspWifiMode(WifiConfig::Mode mode) {
        switch (mode) {
            case WifiConfig::Mode::AccessPoint:
                return WIFI_MODE_AP;
            case WifiConfig::Mode::Station:
                return WIFI_MODE_STA;
            case WifiConfig::Mode::AccessPointAndStation:
                return WIFI_MODE_APSTA;
            default:
                return WIFI_MODE_AP;
        }
    }

    ErrorType initAccessPoint();
    ErrorType initStation();

    ErrorType setStationSsid(const std::string &ssid);
    ErrorType setAccessPointSsid(const std::string &ssid);
};

#endif // __WIFI_MODULE_HPP__