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
        Gsm,              ///< Global System for Mobile Commmunication (2G)
        GsmCompact,       ///< Global System for Mobile Commmunication Compact (2G)
        Utran,            ///< Universal Terrestrial Radio Access Network (3G)
        GsmEgprs,         ///< Global System for Mobile Communications - Enhanced Packet Radio Service
        UtranHsdpa,       ///< Universal Terrestrial Radio Access Network - High-Speed Downlink Packet Access (3G)
        UtranHsupa,       ///< Universal Terrestrial Radio Access Network - High-Speed Uplink Packet Access (3G)
        UtranHsdpaHsupa,  ///< Universal Terrestrial Radio Access Network - High-Speed Downlink and Uplink Packet Access (3G)
        Eutran,           ///< Evolved Universal Terrestrial Radio Access Network (4G)
        UtranHsupaPlus,   ///< Universal Terrestrial Radio Access Network - High-Speed Uplink Packet Access Plus (4G)
        UtranAndEutran    ///< Universal Terrestrial Radio Access Network and Evolved Universal Terrestrial Radio Access Network (3G/4G)
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

    /// @brief The maximum length of an APN
    constexpr size_t MaxApnLength = 64;
    /// @typedef Apn
    /// @brief The Access Point Name (APN) type
    using Apn = std::array<char, MaxApnLength>;

    /**
     * @struct Status
     * @brief The status of the cellular network.
    */
    struct Status {
        DecibelMilliWatts signalStrength; ///< The signal strength of the network interface.
        StaticString::Container carrier;  ///< The carrier that the modem is registered to.
        RadioAccessTechnology rat;        ///< The radio access technology currently in use.

        Status() : signalStrength(0), carrier(std::integral_constant<size_t, 16>()), rat(RadioAccessTechnology::Unknown) {}
    };

    /**
     * @struct CellularParams
     * @brief Contains the parameters used to configure the cellular device.
     */
    struct Params final : public NetworkTypes::ConfigurationParameters {
        /// @brief The technology type these parameters are meant for
        NetworkTypes::Technology technology() const override { return NetworkTypes::Technology::Cellular; }

        Apn apn;                                                    ///< The APN to use for the cellular device
        CellularTypes::RadioAccessTechnology radioAccessTechnology; ///< The radio access technology to use for the cellular device
        CellularTypes::AccessMode accessMode;                       ///< The access mode to use for the cellular device
        IcCommunicationProtocolTypes::IcDevice icDevice;            ///< The IC device to use to communicate with the cellular modem. Do not set if not needed.
        PeripheralNumber resetGpio;                                 ///< The gpio peripheral that contains the reset pin to use for the cellular device. Do not set if not needed.
        PinNumber resetPin;                                         ///< The reset pin to use for the cellular device. Do not set if not needed.
        PeripheralNumber icCommPeripheralNumber;                    ///< The IC communication peripheral to use for the cellular device. Do not set if not needed
        PinNumber icCommTx;                                         ///< The TX pin to use for the cellular device. Do not set if not needed.
        PinNumber icCommRx;                                         ///< The RX pin to use for the cellular device. Do not set if not needed.
        PinNumber icCommRts;                                        ///< The RTS pin to use for the cellular device. Do not set if not needed.
        PinNumber icCommCts;                                        ///< The CTS pin to use for the cellular device. Do not set if not needed.

        Params() : NetworkTypes::ConfigurationParameters() {
            apn.fill(0);
            radioAccessTechnology = CellularTypes::RadioAccessTechnology::Unknown;
            accessMode = CellularTypes::AccessMode::Unknown;
            icDevice = IcCommunicationProtocolTypes::IcDevice::Unknown;
            resetGpio = PeripheralNumber::Unknown;
            resetPin = -1;
            icCommPeripheralNumber = PeripheralNumber::Unknown;
            icCommTx = -1;
            icCommRx = -1;
            icCommRts = -1;
            icCommCts = -1;
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
    CellularAbstraction() : NetworkAbstraction() {
        NetworkAbstraction::_status.technology = NetworkTypes::Technology::Cellular;
    }

    /// @brief Default destructor
    virtual ~CellularAbstraction() = default;

    /// @brief Tag for logging
    static constexpr char TAG[] = "Cellular";

    void printStatus() {
        status();
        PLT_LOGI(TAG, "<CellularStatus> <Carrier:%s, isUp:%d, Signal Strength (dBm):%d> <Pie, Pie, Line>",
                 _status.carrier->c_str(),
                 NetworkAbstraction::_status.isUp,
                 _status.signalStrength);
        }

    /**
     * @brief Reset the cellular device.
     * @returns The error type.
    */
    virtual ErrorType reset() = 0;

    /**
     * @brief Configure the cellular network before initializing
     * @param[in] parameters The parameters to configure with
     * @sa CellularTypes::CellularParams
     * @returns ErrorType::Success
     */
    ErrorType configure(const NetworkTypes::ConfigurationParameters &parameters) override {
        assert(parameters.technology() == NetworkTypes::Technology::Cellular);

        _params = static_cast<const CellularTypes::Params &>(parameters);

        return ErrorType::Success;
    }

    /// @brief Get the cellular parameters as a constant reference
    const CellularTypes::Params &params() const { return _params; }
    /// @brief Get the status of the cellular network
    const CellularTypes::Status &status(const bool updateStatus = true) {
        if (updateStatus) {
            getSignalStrength(_status.signalStrength);
        }

        return _status;
    }

    protected:
    /// @brief The cellular parameters.
    CellularTypes::Params _params;
    /// @brief The status of the cellular network
    CellularTypes::Status _status;
};

#endif // __CELLULAR_ABSTRACTION_HPP__