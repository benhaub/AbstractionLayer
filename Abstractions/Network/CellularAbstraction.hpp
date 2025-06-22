/***************************************************************************//**
* @author   Ben Haubrich
* @file     CellularAbstraction.hpp
* @details  \b Synopsis: \n Interface for communication over the network.
* @ingroup Abstractions
*******************************************************************************/
#ifndef __CELLULAR_ABSTRACTION_HPP__
#define __CELLULAR_ABSTRACTION_HPP__

#include "NetworkAbstraction.hpp"
#include "IcCommunicationProtocol.hpp"

/**
 * @namespace CellularConfig
 * @brief Configuration and types for the cellular network
 */
namespace CellularConfig {

    /**
     * @enum RadioAccessTechnology
     * @brief Cellular RAT
    */
    enum class RadioAccessTechnology : uint8_t {
        Unknown = 0,      ///< Unknown
        Gsm,              ///< Global System for Mobile
        Cdma,             ///< Code-devision multiple access
        Lte,              ///< Long Term Evolution (4G)
        NbIoT,            ///< Narrowband IoT
        GsmAndCdma,       ///< Global System for Mobile and Code-devision multiple access
        GsmAndLte,        ///< Global System for Mobile and Long Term Evolution (3G and 4G)
        CdmaAndLte,       ///< Code-devision multiple access and Long Term Evolution
        GsmAndCdmaAndLte  ///< Global System for Mobile and Code-devision multiple access and Long Term Evolution
    };

    /**
     * @enum AccessMode
     * @brief The access mode used to communicate with the cellular device.
     */
    enum class AccessMode : uint8_t {
        Unknown = 0, ///< Unknown
        Transparent, ///< Transparent. Commands and responses are sent over pure uart.
        Buffer,      ///< Buffer. Commands and responses are sent using AT commands.
        DirectPush   ///< Direct push. Commands are sent using an AT command and responses are sent over pure uart
    };
}

/**
 * @class CellularAbstraction
 * @brief Interface for communication over the cellular network.
*/
class CellularAbstraction : public NetworkAbstraction {

    public:
    /// @brief Default constructor
    CellularAbstraction() {
        _status.isUp = false;
        _status.technology = NetworkTypes::Technology::Cellular;
    }
    /// @brief Default destructor
    virtual ~CellularAbstraction() = default;

    /// @brief Tag for logging
    static constexpr char TAG[] = "Cellular";

    /**
     * @brief Reset the cellular device.
     * @returns The error type.
    */
    virtual ErrorType reset() = 0;
    /**
     * @brief Get the APN.
     * @returns The APN as a mutable reference.
    */
    std::string &accessPointName() { return _accessPointName; }
    /**
     * @brief Get the APN.
     * @returns The APN as a constant reference.
    */
    const std::string &accessPointNameConst() const { return _accessPointName; }
    /**
     * @brief Get the reset pin.
     * @returns The reset pin as a mutable reference.
    */
    PinNumber &resetPin() { return _resetPin; }
    /**
     * @brief Get the reset pin.
     * @returns The reset pin as a constant reference.
    */
    const PinNumber &resetPinConst() const { return _resetPin; }
    /**
     * @brief ic
     * @returns The IC device used to communicate with the RTC as a const reference
     */
    const IcCommunicationProtocol &icConst() const { assert(nullptr != _ic); return *_ic; }
    /**
     * @brief ic
     * @returns The IC device used to communicate with the RTC as a mutable reference
     */
    IcCommunicationProtocol &ic() { assert(nullptr != _ic); return *_ic; }
    /**
     * @brief Get the radio access technology.
     * @returns The radio access technology as a mutable reference.
    */
    CellularConfig::RadioAccessTechnology &radioAccessTechnology() { return _radioAccessTechnology; }
    /**
     * @brief Get the radio access technology.
     * @returns The radio access technology as a constant reference.
    */
    const CellularConfig::RadioAccessTechnology &radioAccessTechnologyConst() const { return _radioAccessTechnology; }
    /**
     * @brief Get the access mode.
     * @returns The access mode as a mutable reference.
    */
    CellularConfig::AccessMode &accessMode() { return _accessMode; }
    /**
     * @brief Get the access mode.
     * @returns The access mode as a constant reference.
    */
    const CellularConfig::AccessMode &accessModeConst() const { return _accessMode; }
    /**
     * @brief Set the IC device used to communicate with the network device.
     * @param ic The IC device to use.
     */
    ErrorType setIcDevice(IcCommunicationProtocol &ic) { _ic = &ic; return ErrorType::Success; }

    ErrorType configure(const NetworkTypes::ConfigurationParameters &parameters) override {
        return ErrorType::NotImplemented;
    }

    protected:
    /// @brief The Access Point Name (APN).
    std::string _accessPointName = std::string();
    /// @brief The pin number for the reset pin.
    PinNumber _resetPin = -1;
    /// @brief The IC peripheral used to communicate with the network device.
    IcCommunicationProtocol *_ic;
    /// @brief The radio access technology.
    CellularConfig::RadioAccessTechnology _radioAccessTechnology = CellularConfig::RadioAccessTechnology::Unknown;
    /// @brief The access mode.
    CellularConfig::AccessMode _accessMode = CellularConfig::AccessMode::Unknown;
};

#endif // __CELLULAR_ABSTRACTION_HPP__