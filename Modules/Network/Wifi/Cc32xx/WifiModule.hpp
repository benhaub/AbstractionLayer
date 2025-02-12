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
    Wifi();
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
    ErrorType setSsid(WifiConfig::Mode mode, const std::string &ssid) override;
    ErrorType setPassword(WifiConfig::Mode mode, const std::string &password) override;
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

    _u8 toCc32xxRole(const WifiConfig::Mode mode, ErrorType &error) {
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

    signed short toCc32xxSecurityType(const WifiConfig::AuthMode authMode, ErrorType &error) {
        error = ErrorType::Success;

        switch (authMode) {
            case WifiConfig::AuthMode::Open:
                return SL_WLAN_SEC_TYPE_OPEN;
            case WifiConfig::AuthMode::Wep:
                return SL_WLAN_SEC_TYPE_WEP;
            case WifiConfig::AuthMode::Wpa:
                return SL_WLAN_SEC_TYPE_WPA;
            case WifiConfig::AuthMode::WpaWpa2:
                return SL_WLAN_SEC_TYPE_WPA_WPA2;
            case WifiConfig::AuthMode::WpsPbc:
                return SL_WLAN_SEC_TYPE_WPS_PBC;
            case WifiConfig::AuthMode::WpsPin:
                return SL_WLAN_SEC_TYPE_WPS_PIN;
            case WifiConfig::AuthMode::WpaEnt:
                return SL_WLAN_SEC_TYPE_WPA_ENT;
            case WifiConfig::AuthMode::P2pPbc:
                return SL_WLAN_SEC_TYPE_P2P_PBC;
            case WifiConfig::AuthMode::P2pPinKeypad:
                return SL_WLAN_SEC_TYPE_P2P_PIN_KEYPAD;
            case WifiConfig::AuthMode::P2pPinDisplay:
                return SL_WLAN_SEC_TYPE_P2P_PIN_DISPLAY;
            case WifiConfig::AuthMode::P2pPinAuto:
                return SL_WLAN_SEC_TYPE_P2P_PIN_AUTO;
            case WifiConfig::AuthMode::WepShared:
                return SL_WLAN_SEC_TYPE_WEP_SHARED;
            case WifiConfig::AuthMode::Wpa2Plus:
                return SL_WLAN_SEC_TYPE_WPA2_PLUS;
            case WifiConfig::AuthMode::Wpa3:
                return SL_WLAN_SEC_TYPE_WPA3;
            case WifiConfig::AuthMode::WpaPmk:
                return SL_WLAN_SEC_TYPE_WPA_PMK;
            default:
                error = ErrorType::InvalidParameter;
                return SL_WLAN_SEC_TYPE_OPEN;
        }
    }
};

#endif // __WIFI_MODULE_HPP__
