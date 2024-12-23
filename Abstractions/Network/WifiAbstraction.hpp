/***************************************************************************//**
* @author   Ben Haubrich
* @file     WifiAbstraction.hpp
* @details  \b Synopsis: \n Interface for communication over the network.
* @ingroup  AbstractionLayer
*******************************************************************************/
#ifndef __WIFI_ABSTRACTION_HPP__
#define __WIFI_ABSTRACTION_HPP__

#include "NetworkAbstraction.hpp"

namespace WifiConfig {

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
        Unknown = 0, ///< Unknown
        Open,        ///< Open
        Wep,         ///< WEP
        WpaPsk,      ///< WPA-PSK
        Wpa2Psk      ///< WPA2-PSK
    };
}

/**
 * @class WifiAbstraction
 * @brief Interface for communication over the wifi.
*/
class WifiAbstraction : public NetworkAbstraction {
    public:
    /**
     * @fn WifiAbstraction
     * @sa WifiFactory::Factory
    */
    WifiAbstraction() = default;
    virtual ~WifiAbstraction() = default;

    /// @brief Tag for logging
    static constexpr char TAG[] = "Wifi";

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
     */
    virtual ErrorType setSsid(WifiConfig::Mode mode, std::string ssid) = 0;
    /**
     * @brief Set the password for the selected mode.
     * @param[in] mode The wifi mode to set the password for
     * @param[in] password The password to set
    */
    virtual ErrorType setPassword(WifiConfig::Mode mode, std::string password) = 0;
    /**
     * @brief set the wifi mode.
     * @param[in] mode The mode to set.
     * @sa WifiConfig::Mode
    */
    virtual ErrorType setMode(WifiConfig::Mode mode) = 0;
    /**
     * @brief set the wifi authentication mode.
     * @param[in] authMode The authentication mode to set.
     * @sa WifiConfig::AuthMode
     */
    virtual ErrorType setAuthMode(WifiConfig::AuthMode authMode) = 0;

    const WifiConfig::Mode &mode() const { return _mode; }
    const std::string &ssid() const { return _ssid; }
    const std::string &password() const { return _password; }
    const uint8_t &channel() const { return _channel; }
    const uint8_t &maxConnections() const { return _maxConnections; }
    const WifiConfig::AuthMode &authMode() const { return _authMode; }
    const std::string &ipAddress() const { return _ipAddress; }

    protected:
    WifiConfig::Mode _mode = WifiConfig::Mode::Unknown;
    std::string _ssid = std::string(32, '\0');
    std::string _password;
    std::string _ipAddress;
    uint8_t _channel = 0;
    uint8_t _maxConnections = 0;
    WifiConfig::AuthMode _authMode = WifiConfig::AuthMode::Unknown;
};

#endif