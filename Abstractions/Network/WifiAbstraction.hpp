/***************************************************************************//**
* @author   Ben Haubrich
* @file     WifiAbstraction.hpp
* @details  \b Synopsis: \n Interface for communication over the network.
* @ingroup Abstractions
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
}

/**
 * @class WifiAbstraction
 * @brief Interface for communication over the wifi.
 * @code
 * //Initialization should be done in this order
 * Wifi wifi; //Call anything that is necessary to allow config to be set
 * wifi.setMode();
 * wifi.setAuthMode();
 * wifi.setSsid();
 * wifi.setPassword();
 * wifi.init(); //Turn on the radio, bring up the network. Network is ready to use.
 * @endcode
*/
class WifiAbstraction : public NetworkAbstraction {
    public:
    /**
     * @brief Anything that should be called before init should be called in the constructor
    */
    WifiAbstraction() {
        _status.isUp = false;
        _status.technology = NetworkTypes::Technology::Wifi;
    }
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
     * @pre Since passwords have length requirements based on the authorization mode, you must call setAuthMode first.
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

    /// @brief  Get the mode as a constant reference
    const WifiConfig::Mode &mode() const { return _mode; }
    /// @brief Get the SSID as a constant reference
    const std::string &ssid() const { return _ssid; }
    /// @brief Get the password as a constant reference
    const std::string &password() const { return _password; }
    /// @brief Get the channel as a constant reference
    const uint8_t &channel() const { return _channel; }
    /// @brief Get the max connections as a constant reference
    const uint8_t &maxConnections() const { return _maxConnections; }
    /// @brief Get the authentication mode as a constant reference
    const WifiConfig::AuthMode &authMode() const { return _authMode; }
    /// @brief Get the ip address as a constant reference
    const std::string &ipAddress() const { return _ipAddress; }

    protected:
    /// @brief The current wifi mode
    WifiConfig::Mode _mode = WifiConfig::Mode::Unknown;
    /// @brief The current ssid
    std::string _ssid = std::string(32, '\0');
    /// @brief The current password
    std::string _password;
    /// @brief The current ip address
    std::string _ipAddress;
    /// @brief The current channel
    uint8_t _channel = 0;
    /// @brief The current limit on the number of connections
    uint8_t _maxConnections = 0;
    /// @brief The current authentication mode
    WifiConfig::AuthMode _authMode = WifiConfig::AuthMode::Unknown;
};

#endif