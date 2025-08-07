/***************************************************************************//**
* @author  Ben Haubrich
* @file    CellularAbstraction.hpp
* @details Interface for communication over the network.
* @ingroup Abstractions
*******************************************************************************/
#ifndef __CELLULAR_ABSTRACTION_HPP__
#define __CELLULAR_ABSTRACTION_HPP__

#include "NetworkAbstraction.hpp"
#include "IcCommunicationProtocol.hpp"
#include "GpioAbstraction.hpp"

/**
 * @namespace CellularTypes
 * @brief Configuration and types for the cellular network
 */
namespace CellularTypes {

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

    /**
     * @struct CellularParams
     * @brief Contains the parameters used to configure the cellular device.
     */
    struct CellularParams final : public NetworkTypes::ConfigurationParameters {
        /// @brief The technology type these parameters are meant for
        NetworkTypes::Technology technology() const override { return NetworkTypes::Technology::Cellular; }

        std::array<char, 64> apn;                                   ///< The APN to use for the cellular device
        CellularTypes::RadioAccessTechnology radioAccessTechnology; ///< The radio access technology to use for the cellular device
        CellularTypes::AccessMode accessMode;                       ///< The access mode to use for the cellular device
        IcCommunicationProtocolTypes::IcDevice icDevice;            ///< The IC device to use to communicate with the cellular modem. Do not set if not needed.
        PinNumber resetPin;                                         ///< The reset pin to use for the cellular device. Do not set if not needed.
        PeripheralNumber gpio;                                      ///< The gpio peripheral that contains the reset pin to use for the cellular device. Do not set if not needed.

        CellularParams() : NetworkTypes::ConfigurationParameters() {
            apn.fill(0);
            radioAccessTechnology = CellularTypes::RadioAccessTechnology::Unknown;
            accessMode = CellularTypes::AccessMode::Unknown;
            icDevice = IcCommunicationProtocolTypes::IcDevice::Unknown;
            resetPin = -1;
            gpio = PeripheralNumber::Unknown;
        }
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

    ErrorType configure(const NetworkTypes::ConfigurationParameters &parameters) override {
        assert(parameters.technology() == NetworkTypes::Technology::Cellular);

        _cellularParams = static_cast<const CellularTypes::CellularParams &>(parameters);

        return ErrorType::Success;
    }

    /// @brief Get the cellular parameters as a constant reference
    const CellularTypes::CellularParams &cellularParams() const { return _cellularParams; }

    private:
    // @brief The cellular parameters.
    CellularTypes::CellularParams _cellularParams;
};

#endif // __CELLULAR_ABSTRACTION_HPP__