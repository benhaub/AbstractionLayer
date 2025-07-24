/***************************************************************************//**
* @author   Ben Haubrich
* @file     IcCommFactory.hpp
* @details  Factory for creating various types of IC communication protocols
* @ingroup Applications
*******************************************************************************/
#ifndef __IC_COMM_FACTORY_HPP__
#define __IC_COMM_FACTORY_HPP__

//AbstractionLayer
#include "SpiModule.hpp"
#include "UartModule.hpp"
#include "I2cModule.hpp"
//C++
#include <variant>
#include <optional>

/**
 * @namespace IcCommFactoryTypes
 * @brief Contains types and constants used by the IcCommFactory.
 */
namespace IcCommFactoryTypes {
    ///@brief The tag used for logging
    static constexpr char Tag[] = "IcCommFactory";

    using IcCommFactoryVariant = std::variant<Uart, Spi, I2c>;
}

/**
 * @namespace IcCommFactory
 * @brief Functions and types that aid the creation of IC communication protocols.
 */
namespace IcCommFactory {

    /**
     * @brief Creates a IC device for the type selected.
     * @param device The IC device to use for communication.
     * @sa IcCommunicationProtocolTypes::IcDevice
     * @param params The parameters to use for the IC device.
     * @sa IcCommFactoryTypes::I2cParams
     * @param error The error code following the return of this function.
     * @returns Pointer to an IcCommunicationProtocol that contains the IC device of the type selected.
     */
    inline ErrorType Factory(IcCommunicationProtocolTypes::IcDevice device, std::optional<IcCommFactoryTypes::IcCommFactoryVariant> &ic) {
        ErrorType error = ErrorType::Success;

        switch (device) {
            case IcCommunicationProtocolTypes::IcDevice::Uart:
                ic.emplace(std::in_place_type_t<Uart>());
                break;
            case IcCommunicationProtocolTypes::IcDevice::Spi:
                ic.emplace(std::in_place_type_t<Spi>());
                break;
            case IcCommunicationProtocolTypes::IcDevice::I2c:
                ic.emplace(std::in_place_type_t<I2c>());
                break;
            default: [[unlikely]]
                error =  ErrorType::NotSupported;
        }

        return error;
    }
}

#endif // __NETWORK_FACTORY_HPP__