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

    constexpr size_t MaxApnLength = 64;
    using Apn = std::array<char, MaxApnLength>;

    /**
     * @struct Status
     * @brief The status of the cellular network.
    */
    struct Status {
        DecibelMilliWatts signalStrength;         ///< The signal strength of the network interface.
        StaticString::Container manufacturerName; ///< The manufacturer name of the network interface.

        Status() : signalStrength(0), manufacturerName(StaticString::Data<16>()) {}
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
        PLT_LOGI(TAG, "<CellularStatus> <Technology:%u, isUp:%d, Signal Strength (dBm):%d> <Omit, Pie, Line>",
        static_cast<uint8_t>(NetworkAbstraction::_status.technology),
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
            //Not only does it not really make sense to want to know the signal strength if you aren't connected to anything,
            //but some platforms will not allow this and may event crash if you ask for the status before the network is initialized.
            if (NetworkAbstraction::_status.isUp) {
                getSignalStrength(_status.signalStrength);
            }
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