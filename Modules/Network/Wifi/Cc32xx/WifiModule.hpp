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

class Wifi final : public WifiAbstraction {

    public:
    Wifi();

    ErrorType init() override;
    ErrorType networkUp() override;
    ErrorType networkDown() override;
    ErrorType txBlocking(const std::string &frame, const Socket socket, const Milliseconds timeout) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> frameBuffer, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> frameBuffer)> callback) override;
    ErrorType getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) override;
    ErrorType getSignalStrength(DecibelMilliWatts &signalStrength) override;

    ErrorType radioOn() override;
    ErrorType radioOff() override;
    ErrorType setSsid(WifiTypes::Mode mode, const std::string &ssid) override;
    ErrorType setPassword(WifiTypes::Mode mode, const std::string &password) override;
    ErrorType setMode(WifiTypes::Mode mode) override;
    ErrorType setAuthMode(WifiTypes::AuthMode authMode) override;

    private:
    WifiTypes::Mode fromCc32xxRole(const _i16 role) {
        switch (role) {
            case ROLE_STA:
                return WifiTypes::Mode::Station;
            case ROLE_AP:
                return WifiTypes::Mode::AccessPoint;
            case ROLE_P2P:
                return WifiTypes::Mode::PointToPoint;
            case ROLE_TAG:
            default:
                return WifiTypes::Mode::Unknown;
        }
    }

    _u8 toCc32xxRole(const WifiTypes::Mode mode, ErrorType &error) {
        error = ErrorType::Success;

        switch (mode) {
            case WifiTypes::Mode::Station:
            case WifiTypes::Mode::AccessPointAndStation:
                return ROLE_STA;
            case WifiTypes::Mode::AccessPoint:
                return ROLE_AP;
            case WifiTypes::Mode::PointToPoint:
                return ROLE_P2P;
            case WifiTypes::Mode::Unknown:
            default:
                error = ErrorType::InvalidParameter;
                return ROLE_STA;
        }
    }

    _u8 toCc32xxSecurityType(const WifiTypes::AuthMode authMode, ErrorType &error) {
        error = ErrorType::Success;

        switch (authMode) {
            case WifiTypes::AuthMode::Open:
                return SL_WLAN_SEC_TYPE_OPEN;
            case WifiTypes::AuthMode::Wep:
                return SL_WLAN_SEC_TYPE_WEP;
            case WifiTypes::AuthMode::Wpa:
                return SL_WLAN_SEC_TYPE_WPA;
            case WifiTypes::AuthMode::WpaWpa2:
                return SL_WLAN_SEC_TYPE_WPA_WPA2;
            case WifiTypes::AuthMode::WpsPbc:
                return SL_WLAN_SEC_TYPE_WPS_PBC;
            case WifiTypes::AuthMode::WpsPin:
                return SL_WLAN_SEC_TYPE_WPS_PIN;
            case WifiTypes::AuthMode::WpaEnt:
                return SL_WLAN_SEC_TYPE_WPA_ENT;
            case WifiTypes::AuthMode::P2pPbc:
                return SL_WLAN_SEC_TYPE_P2P_PBC;
            case WifiTypes::AuthMode::P2pPinKeypad:
                return SL_WLAN_SEC_TYPE_P2P_PIN_KEYPAD;
            case WifiTypes::AuthMode::P2pPinDisplay:
                return SL_WLAN_SEC_TYPE_P2P_PIN_DISPLAY;
            case WifiTypes::AuthMode::P2pPinAuto:
                return SL_WLAN_SEC_TYPE_P2P_PIN_AUTO;
            case WifiTypes::AuthMode::WepShared:
                return SL_WLAN_SEC_TYPE_WEP_SHARED;
            case WifiTypes::AuthMode::Wpa2Plus:
                return SL_WLAN_SEC_TYPE_WPA2_PLUS;
            case WifiTypes::AuthMode::Wpa3:
                return SL_WLAN_SEC_TYPE_WPA3;
            case WifiTypes::AuthMode::WpaPmk:
                return SL_WLAN_SEC_TYPE_WPA_PMK;
            default:
                error = ErrorType::InvalidParameter;
                return SL_WLAN_SEC_TYPE_OPEN;
        }
    }

    _u8 toCc32xxProvisioningMode(const WifiTypes::Mode &mode, ErrorType &error) {
        error = ErrorType::Success;

        switch (mode) {
            case WifiTypes::Mode::AccessPoint:
                return SL_WLAN_PROVISIONING_CMD_START_MODE_AP;
            case WifiTypes::Mode::Station:
                return SL_WLAN_PROVISIONING_CMD_START_MODE_SC;
            case WifiTypes::Mode::AccessPointAndStation:
                return SL_WLAN_PROVISIONING_CMD_START_MODE_APSC_EXTERNAL_CONFIGURATION;
            default:
                error = ErrorType::InvalidParameter;
                return SL_WLAN_PROVISIONING_CMD_STOP;
        }
    }
};

#endif // __WIFI_MODULE_HPP__
