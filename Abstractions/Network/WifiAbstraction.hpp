/***************************************************************************//**
* @author   Ben Haubrich
* @file     WifiAbstraction.hpp
* @details  \b Synopsis: \n Interface for communication over the network.
* @ingroup Abstractions
*******************************************************************************/
#ifndef __WIFI_ABSTRACTION_HPP__
#define __WIFI_ABSTRACTION_HPP__

#include "NetworkAbstraction.hpp"

/**
 * @namespace WifiTypes
 * @brief Types for the wifi abstraction.
*/
namespace WifiTypes {

    /**
     * @enum Mode
     * @brief Wifi mode that effictively says whether you are connecting to wifi or being connected to by some other device.
    */
    enum class Mode : uint8_t {
        Unknown = 0,
        AccessPoint,           ///< Access Point. You can connect to these from a station.
        Station,               ///< Station. You can connect these to an access point.
        AccessPointAndStation, ///< Access Point and Station.
        PointToPoint           ///< Point to Point. Connects two devices directly without any hosts or other networking in between.
    };

    /**
     * @enum AuthMode
     * @brief Wifi authentication mode.
    */
    enum class AuthMode : uint8_t {
        Unknown = 0,   ///< Unknown
        Open,          ///< Open
        Wep,           ///< WEP
        Wpa,           ///< WPA
        WpaWpa2,       ///< WPA/WPA2
        WpsPbc,        ///<WPS/PBC
        WpsPin,        ///< WPS/PIN
        WpaEnt,        ///< WPA Enterprise
        P2pPbc,        ///< P2P Push Button Configuration
        P2pPinKeypad,  ///< P2P Pin Keypad
        P2pPinDisplay, ///< P2P Pin Display
        P2pPinAuto,    ///< P2P Pin Auto
        WepShared,     ///< WEP Shared
        Wpa2Plus,      ///< WPA2 Plus
        Wpa3,          ///< WPA3
        WpaPmk         ///< WPA PMK
    };

    constexpr size_t MaxSsidLength = 32;
    constexpr size_t MaxPasswordLength = 64;

    /**
     * @struct Status
     * @brief The status of the wifi network.
    */
    struct Status {
        bool isProvisioned; ///< True when the network is provisioned and ready to connect to access points.
        DecibelMilliWatts signalStrength;///< The signal strength of the network interface.

        Status() : isProvisioned(false), signalStrength(0) {}
    };

    /**
     * @struct WifiParams
     * @brief Contains the parameters used to configure the wifi.
     */
    struct WifiParams final : public NetworkTypes::ConfigurationParameters {
        /// @brief The technology type these parameters are meant for
        NetworkTypes::Technology technology() const override { return NetworkTypes::Technology::Wifi; }

        std::array<char, MaxSsidLength> accessPointSsid;         ///< The SSID of the access point (Connecting to this device)
        std::array<char, MaxPasswordLength> accessPointPassword; ///< The password of the access point
        std::array<char, MaxSsidLength> stationSsid;             ///< The SSID of the station (Wifi that this device connects to)
        std::array<char, MaxPasswordLength> stationPassword;     ///< The password of the station
        WifiTypes::AuthMode authMode;                            ///< Password authentication
        WifiTypes::Mode mode;                                    ///< Station, access point, or both
    };
}

/**
 * @class WifiAbstraction
 * @brief Interface for communication over the wifi.
*/
class WifiAbstraction : public NetworkAbstraction {

    public:
    /**
     * @brief Anything that should be called before init should be called in the constructor
    */
    WifiAbstraction() {
        _status.isProvisioned = false;
        NetworkAbstraction::_status.technology = NetworkTypes::Technology::Wifi;
    }
    virtual ~WifiAbstraction() = default;

    /// @brief Tag for logging
    static constexpr char TAG[] = "Wifi";

    void printStatus() {
        status();
        PLT_LOGI(TAG, "<WifiStatus> <Technology:%u, isUp:%s, isProvisioned:%s, Signal Strength (dBm):%d> <Pie, Line>",
        static_cast<uint8_t>(NetworkAbstraction::_status.technology),
                             _status.isProvisioned ? "yes" : "no",
                             NetworkAbstraction::_status.isUp ? "true" : "false",
                             _status.signalStrength);
    }

    /**
     * @brief Turn the wifi radio on.
     * @return ErrorType::Success if the radio was turned on successfully
     * @return ErrorType::Failure if the radio was not turned on successfully
    */
    virtual ErrorType radioOn() = 0;
    /**
     * @brief Turn the wifi radio off.
     * @return ErrorType::Success if the radio was turned off successfully
     * @return ErrorType::Failure if the radio was not turned off successfully
    */
    virtual ErrorType radioOff() = 0;

    /**
     * @brief Set the ssid for the selected mode.
     * @param[in] mode The wifi mode to set the SSID for
     * @param[in] ssid The ssid to set
     * @returns ErrorType::InvalidParameter if the ssid is longer thatn 32 bytes.
     * @returns ErrorType::InvalidParameter if the mode is AccessPointAndStation.
     * @returns ErrorType::Success if the ssid is set.
     * @post No changes take effect until either initialization is complete or wifi is reinitialized
     */
    virtual ErrorType setSsid(WifiTypes::Mode mode, const std::string &ssid) = 0;
    /**
     * @brief Set the password for the selected mode.
     * @pre Since passwords have length requirements based on the authorization mode, you must call setAuthMode first.
     * @param[in] mode The wifi mode to set the password for
     * @param[in] password The password to set
     * @returns ErrorType::InvalidParameter if the password is longer than 64 bytes.
     * @returns ErrorType::InvalidParameter if the mode is AccessPointAndStation.
     * @returns ErrorType::Success if the password is set.
     * @post No changes take effect until either initialization is complete or wifi is reinitialized
    */
    virtual ErrorType setPassword(WifiTypes::Mode mode, const std::string &password) = 0;
    /**
     * @brief set the wifi mode.
     * @param[in] mode The mode to set.
     * @sa WifiConfig::Mode
     * @post No changes take effect until either initialization is complete or wifi is reinitialized
    */
    virtual ErrorType setMode(WifiTypes::Mode mode) = 0;
    /**
     * @brief set the wifi authentication mode.
     * @param[in] authMode The authentication mode to set.
     * @sa WifiConfig::AuthMode
     * @post No changes take effect until either initialization is complete or wifi is reinitialized
     */
    virtual ErrorType setAuthMode(WifiTypes::AuthMode authMode) = 0;

    ErrorType configure(const NetworkTypes::ConfigurationParameters &parameters) override {
        assert(parameters.technology() == NetworkTypes::Technology::Wifi);
        ErrorType error = ErrorType::Success;

        const auto &params = static_cast<const WifiTypes::WifiParams &>(parameters);

        if (WifiTypes::Mode::AccessPointAndStation == params.mode) {
            error = setSsid(WifiTypes::Mode::AccessPoint, params.accessPointSsid.data());
            if (ErrorType::Success == error) {
                error = setSsid(WifiTypes::Mode::Station, params.stationSsid.data());
            }
        }
        else if (WifiTypes::Mode::AccessPoint == params.mode) {
            error = setSsid(WifiTypes::Mode::AccessPoint, params.accessPointSsid.data());
        }
        else if (WifiTypes::Mode::Station == params.mode) {
            error = setSsid(WifiTypes::Mode::Station, params.stationSsid.data());
        }
        if (ErrorType::NotImplemented == error || ErrorType::NotAvailable == error) {
            PLT_LOGW(TAG, "Setting wifi SSID is not allowed on this platform <error:%u>", (uint8_t)error);
        }
        else if (ErrorType::Success != error) {
            PLT_LOGE(TAG, "Failed to set ssid <error:%u>", (uint8_t)error);
        }

        error = setAuthMode(params.authMode);
        if (ErrorType::NotImplemented == error || ErrorType::NotAvailable == error) {
            PLT_LOGW(TAG, "Setting authorization modes is not allowed on this platform <error:%u>", (uint8_t)error);
        }
        else if (ErrorType::Success != error) {
            PLT_LOGE(TAG, "Failed to set atuhorization mode <error:%u>", (uint8_t)error);
        }

        if (WifiTypes::Mode::AccessPointAndStation == params.mode) {
            error = setPassword(WifiTypes::Mode::AccessPoint, params.accessPointPassword.data());
            if (ErrorType::Success == error) {
                error = setPassword(WifiTypes::Mode::Station, params.stationPassword.data());
            }
        }
        else if (WifiTypes::Mode::AccessPoint == params.mode) {
            error = setPassword(WifiTypes::Mode::AccessPoint, params.accessPointPassword.data());
        }
        else if (WifiTypes::Mode::Station == params.mode) {
            error = setPassword(WifiTypes::Mode::Station, params.stationPassword.data());
        }
        if (ErrorType::NotImplemented == error || ErrorType::NotAvailable == error) {
            PLT_LOGW(TAG, "Setting wifi password is not allowed on this platform <error:%u>", (uint8_t)error);
        }
        else if (ErrorType::Success != error) {
            PLT_LOGE(TAG, "Failed to set password <error:%u>", (uint8_t)error);
        }

        error = setMode(params.mode);
        if (ErrorType::Success != error) {
            const bool isCriticalErrror = !((ErrorType::NotImplemented == error) || (ErrorType::NotAvailable == error));
            if (isCriticalErrror) {
                PLT_LOGE(TAG, "Failed to set wifi mode <error:%u>", (uint8_t)error);
                return error;
            }
            else {
                PLT_LOGW(TAG, "Setting wifi modes is not allowed on this platform <error:%u>", (uint8_t)error);
            }
        }

        return error;
    }

    /**
     * @brief Update the wifi credentials.
     * @param[in] mode The wifi mode to update the credentials for.
     * @param[in] ssid The SSID to update to.
     * @param[in] password The password to update to.
     * @returns ErrorType::InvalidParameter if the mode is AccessPointAndStation. Update one mode at a time.
     * @returns ErrorType::Failure if the network could not be brought down or back up.
     * @returns ErrorType::Success if the credentials were updated successfully.
     * @post This function brings the network down and back up. Any existing connections will be terminated.
     */
    ErrorType updateWifiCredentials(WifiTypes::Mode mode, std::string_view ssid, std::string_view password) {
        ErrorType error = ErrorType::Failure;

        error = networkDown();

        if (ErrorType::Success == error) {
            if (mode == WifiTypes::Mode::AccessPoint) {
                _accessPointSsid = ssid;
                _accessPointPassword = password;
            }
            else if (mode == WifiTypes::Mode::Station) {
                _stationSsid = ssid;
                _stationPassword = password;
            }
            else {
                error = ErrorType::InvalidParameter;
            }

            if (ErrorType::Success == error) {
                error = networkUp();
            }
        }

        return error;
    }

    /// @brief  Get the mode as a constant reference
    const WifiTypes::Mode &mode() const { return _mode; }
    /// @brief Get the SSID for the access point as a constant reference
    const std::string &accessPointSsid() const { return _accessPointSsid; }
    /// @brief Get the password for the access point as a constant reference
    const std::string &accessPointPassword() const { return _accessPointPassword; }
    /// @brief Get the SSID for the station as a constant reference
    const std::string &stationSsid() const { return _stationSsid; }
    /// @brief Get the password for the station as a constant reference
    const std::string &stationPassword() const { return _stationPassword; }
    /// @brief Get the channel as a constant reference
    const uint8_t &channel() const { return _channel; }
    /// @brief Get the max connections as a constant reference
    const uint8_t &maxConnections() const { return _maxConnections; }
    /// @brief Get the authentication mode as a constant reference
    const WifiTypes::AuthMode &authMode() const { return _authMode; }
    /// @brief Get the status of the wifi network
    const WifiTypes::Status &status(const bool updateStatus = true) {
        if (updateStatus) {
            //Not only does it not really make sense to want to know the signal strength if you aren't connected to anything,
            //but some platforms will not allow this and may event crash if you ask for the status before the network is initialized.
            if (NetworkAbstraction::_status.isUp) {
                getSignalStrength(_status.signalStrength);
            }
        }

        return _status;
    }

    protected:
    /// @brief The current wifi mode
    WifiTypes::Mode _mode = WifiTypes::Mode::Unknown;
    /// @brief The current access point ssid
    std::string _accessPointSsid;
    /// @brief The current access point password
    std::string _accessPointPassword;
    /// @brief The current station ssid
    std::string _stationSsid;
    /// @brief The current station password
    std::string _stationPassword;
    /// @brief The current channel
    uint8_t _channel = 0;
    /// @brief The current limit on the number of connections
    uint8_t _maxConnections = 0;
    /// @brief The current authentication mode
    WifiTypes::AuthMode _authMode = WifiTypes::AuthMode::Unknown;
    /// @brief Status of the wifi network
    WifiTypes::Status _status;
};

#endif