/***************************************************************************//**
* @author   Ben Haubrich
* @file     NetworkFactory.hpp
* @details  Network abstraction factory for creating various types of network interfaces
* @ingroup Applications
* @sa https://en.wikipedia.org/wiki/Factory_(object-oriented_programming)
*******************************************************************************/
#ifndef __NETWORK_FACTORY_HPP__
#define __NETWORK_FACTORY_HPP__

//AbstractionLayer
#include "WifiModule.hpp"
#include "CellularModule.hpp"
//C++
#include <variant>
#include <optional>

/**
 * @namespace NetworkFactoryTypes
 * @brief Contains types and constants used by the NetworkFactory.
 */
namespace NetworkFactoryTypes {

    ///@brief The tag used for logging
    static constexpr char Tag[] = "NetworkFactory";

    /**
     * @typedef NetworkFactoryVariant
     * @brief The variant type that can hold any network type created by the factory.
     */
    using NetworkFactoryVariant = std::variant<Wifi, Cellular>;
}

/**
 * @namespace NetworkFactory
 * @brief Functions and types that aid the creation of network interfaces.
 */
namespace NetworkFactory {

    /**
     * @brief Creates a network interface for the type selected.
     * @details The output network is passed by reference as a parameter because the atomics in EventQueue delete
     *          the move assignment.
     * @param[in] technology The technology to use for the network interface.
     * @sa NetworkTypes::Technology
     * @param[out] network The network that is output by the factory
     * @returns ErrorType::Success if the network was created successfully
     * @returns ErrorType::NotSupported if the technology is not supported
     */
    template <NetworkTypes::Technology _Technology>
    inline constexpr ErrorType Factory(std::optional<NetworkFactoryTypes::NetworkFactoryVariant> &network) {
        ErrorType error = ErrorType::Success;

        if constexpr (_Technology == NetworkTypes::Technology::Wifi) {
            network.emplace(std::in_place_type_t<Wifi>());
        }
        else if constexpr (_Technology == NetworkTypes::Technology::Zigbee) {
            error = ErrorType::NotSupported;
        }
        else if constexpr (_Technology == NetworkTypes::Technology::Ethernet) {
            error = ErrorType::NotSupported;
        }
        else if constexpr (_Technology == NetworkTypes::Technology::Cellular) {
            network.emplace(std::in_place_type_t<Cellular>());
        }
        else { [[unlikely]]
            error =  ErrorType::NotSupported;
        }

        return error;
    }
}

#endif // __NETWORK_FACTORY_HPP__
