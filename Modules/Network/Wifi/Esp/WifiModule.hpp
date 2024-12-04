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
    Wifi() : WifiAbstraction() {
        _status.isUp = false;
        _status.technology = NetworkTypes::Technology::Wifi;
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

    /**
     * @brief Set the ssid for the selected mode.
     * @param[in] mode The wifi mode to set the SSID for
     * @param[in] ssid The ssid to set
     */
    ErrorType setSsid(WifiConfig::Mode mode, std::string ssid) override;
    /**
     * @brief Set the password for the selected mode.
     * @param[in] mode The wifi mode to set the password for
     * @param[in] password The password to set
    */
    ErrorType setPassword(WifiConfig::Mode mode, std::string password) override;
    ///@brief set the wifi mode.
    ErrorType setMode(WifiConfig::Mode mode) override { _mode = mode; return ErrorType::Success; }
    /**
     * @brief set the wifi authentication mode.
     * @param[in] authMode The authentication mode to set.
     * @sa WifiConfig::AuthMode
     */
    ErrorType setAuthMode(WifiConfig::AuthMode authMode) override { _authMode = authMode; return ErrorType::Success; }

    private:
    wifi_auth_mode_t toEspAuthMode(WifiConfig::AuthMode authMode) {
        switch (authMode) {
            case WifiConfig::AuthMode::Open:
                return WIFI_AUTH_OPEN;
            case WifiConfig::AuthMode::Wep:
                return WIFI_AUTH_WEP;
            case WifiConfig::AuthMode::WpaPsk:
                return WIFI_AUTH_WPA_PSK;
            case WifiConfig::AuthMode::Wpa2Psk:
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

    ErrorType setStationSsid(std::string ssid);
    ErrorType setAccessPointSsid(std::string ssid);
};

#endif // __WIFI_MODULE_HPP__