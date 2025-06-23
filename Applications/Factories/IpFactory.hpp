/***************************************************************************//**
* @author   Ben Haubrich
* @file     IpFactory.hpp
* @details  IP abstraction factory for creating various types of Internet Protocol interfaces
* @ingroup Applications
* @sa https://en.wikipedia.org/wiki/Factory_(object-oriented_programming)
*******************************************************************************/
#ifndef __IP_FACTORY_HPP__
#define __IP_FACTORY_HPP__

//AbstractionLayer
#include "IpClientModule.hpp"
#include "IpCellularClientModule.hpp"
#include "IpServerModule.hpp"
#include "IpCellularServerModule.hpp"
#include "HttpServerModule.hpp"
#include "Log.hpp"
#include "NetworkFactory.hpp"
//C++
#include <cassert>
#include <variant>
#include <optional>

///@brief The tag used for logging
static constexpr char IpFactoryTag[] = "IpFactoryTag";

#if _GLIBCXX_CONCEPTS || _LIBCPP_STD_VER >= 20
/// @concept IpClientFactoryRequirements
/// @brief A concept that checks if a type is derived from IpClientAbstraction
template<typename T>
concept IpClientFactoryRequirements = std::derived_from<T, IpClientAbstraction>;

/// @concept IpServerFactoryRequirements
/// @brief A concept that checks if a type is derived from IpServerAbstraction
template<typename T>
concept IpServerFactoryRequirements = std::derived_from<T, IpServerAbstraction>;
#endif

/**
 * @namespace IpFactoryTypes
 * @brief Types and constants used by the IpFactory
 */
namespace IpFactory {

    /**
     * @enum Protocol
     * @brief The protocol used by the IP client at the application layer
    */
    enum class Protocol : uint8_t {
        Unknown = 0, ///< Unknown protocol
        TcpOrUdp,    ///< Raw TCP or UDP with no application or presentation layer support.
        Http,        ///< Hyper Text Transfer Protocol (TCP only)
        Websocket,   ///< Websockets
        Ftp,         ///< File Transfer Protocol (TCP only)
        Mqtt,        ///< Message Queuing Telemetry Transport (TCP only)
        Modbus       ///< Modbus
    };

    using ClientFactoryVariant = std::variant<IpClient, IpCellularClient>;
    using ServerFactoryVariant = std::variant<IpServer, IpCellularServer>;

    /**
     * @brief This factory function creates an IpClientAbstraction that is compatible with the network interface selected
     * @param[in] technology Sometimes the internals of a client will be very different depending on the technology used. For example, Cellular
     *        clients must communicate with the device using a serial interface like SPI, I2C, UART, USB, etc.
     * @sa NetworkTypes::Technology
     * @param[out] client The client that the factory outputs
     * @returns ErrorType::Success if a client could be created.
     * @returns ErrorType::NotSupported If the client can't be created on this platform
     * @post If return value is not ErrorType::Success, client.has_value() is false
     */
    inline ErrorType ClientFactory(NetworkTypes::Technology technology, std::optional<ClientFactoryVariant> &client) {
        ErrorType error = ErrorType::Success;

        switch (technology) {
            case NetworkTypes::Technology::Wifi:
            case NetworkTypes::Technology::Zigbee:
            case NetworkTypes::Technology::Ethernet:
                client.emplace(std::in_place_type_t<IpClient>());
                break;
            case NetworkTypes::Technology::Cellular:
                client.emplace(std::in_place_type_t<IpCellularClient>());
                break;
            case NetworkTypes::Technology::Unknown:
            default: [[unlikely]]
                return ErrorType::NotSupported;
        }

        return error;
    }
    /**
     * @brief This factory function creates an IpServerAbstraction that is compatible with the network interface selected
     * @param[in] technology The network technology that this server will communicate on.
     * @sa NetworkTypes::Technology
     * @param[out] server The server that the factory outputs.
     * @returns ErrorType::Success if a server could be created.
     * @returns ErrorType::NotSupported If the server can't be created on this platform
     * @post If return value is not ErrorType::Success, server.has_value() is false
     */
    inline ErrorType ServerFactory(NetworkTypes::Technology technology, std::optional<ServerFactoryVariant> &server) {
        ErrorType error = ErrorType::Success;

        switch (technology) {
            case NetworkTypes::Technology::Wifi:
            case NetworkTypes::Technology::Ethernet:
            case NetworkTypes::Technology::Zigbee:
                server.emplace(std::in_place_type_t<IpServer>());
                break;
            case NetworkTypes::Technology::Cellular:
                server.emplace(std::in_place_type_t<IpCellularServer>());
                break;
            case NetworkTypes::Technology::Unknown:
            default: [[unlikely]]
                error = ErrorType::NotSupported;
        }

        return error;
    }
}

#endif // __CELLULAR_IP_FACTORY_HPP__