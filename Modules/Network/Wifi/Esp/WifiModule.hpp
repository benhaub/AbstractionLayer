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

    ErrorType mainLoop() override;

    ErrorType radioOn() override;
    ErrorType radioOff() override;
    ErrorType setSsid(WifiTypes::Mode mode, const std::string &ssid) override;
    ErrorType setPassword(WifiTypes::Mode mode, const std::string &password) override;
    ErrorType setMode(WifiTypes::Mode mode) override { _mode = mode; return ErrorType::Success; }
    ErrorType setAuthMode(WifiTypes::AuthMode authMode) override { _authMode = authMode; return ErrorType::Success; }

    private:
    wifi_auth_mode_t toEspAuthMode(WifiTypes::AuthMode authMode) {
        switch (authMode) {
            case WifiTypes::AuthMode::Open:
                return WIFI_AUTH_OPEN;
            case WifiTypes::AuthMode::Wep:
                return WIFI_AUTH_WEP;
            case WifiTypes::AuthMode::Wpa:
                return WIFI_AUTH_WPA_PSK;
            case WifiTypes::AuthMode::WpaWpa2:
                return WIFI_AUTH_WPA2_PSK;
            default:
                return WIFI_AUTH_OPEN;
        }
    }

    wifi_mode_t toEspWifiMode(WifiTypes::Mode mode) {
        switch (mode) {
            case WifiTypes::Mode::AccessPoint:
                return WIFI_MODE_AP;
            case WifiTypes::Mode::Station:
                return WIFI_MODE_STA;
            case WifiTypes::Mode::AccessPointAndStation:
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