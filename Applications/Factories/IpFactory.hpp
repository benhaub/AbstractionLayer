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
#include <concepts>

///@brief The tag used for logging
static constexpr char IpFactoryTag[] = "IpFactoryTag";

/// @concept IpClientFactoryRequirements
/// @brief A concept that checks if a type is derived from IpClientAbstraction
template<typename T>
concept IpClientFactoryRequirements = std::derived_from<T, IpClientAbstraction>;

/// @concept IpServerFactoryRequirements
/// @brief A concept that checks if a type is derived from IpServerAbstraction
template<typename T>
concept IpServerFactoryRequirements = std::derived_from<T, IpServerAbstraction>;

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

    /**
     * @brief This factory function creates an IpClientAbstraction that is compatible with the network interface selected
     * @param[in] technology Sometimes the internals of a client will be very different depending on the technology used. For example, Cellular
     *        clients must communicate with the device using a serial interface like SPI, I2C, UART, USB, etc.
     * @param[in] protocol The protocol that the client should use.
     * @sa NetworkTypes::Technology
     * @param error
     * @sa ErrorType
     */
    inline std::variant<IpClient, IpCellularClient> ClientFactory(NetworkTypes::Technology technology, IpFactory::Protocol protocol, ErrorType &error) {
        static_assert(IpClientFactoryRequirements<IpClient> && IpClientFactoryRequirements<IpCellularClient>, "Variant types must be derived from IpClientAbstraction");

        switch (technology) {
            case NetworkTypes::Technology::Wifi:
                return std::variant<IpClient, IpCellularClient>(std::in_place_type<IpClient>);
            case NetworkTypes::Technology::Zigbee:
            case NetworkTypes::Technology::Ethernet:
            case NetworkTypes::Technology::Cellular:
                return std::variant<IpClient, IpCellularClient>(std::in_place_type<IpCellularClient>);
            case NetworkTypes::Technology::Unknown:
            default: [[unlikely]]
                assert(false);
                return std::variant<IpClient, IpCellularClient>(std::in_place_type<IpClient>);
        }
    }
    /**
     * @brief This factory function creates an IpServerAbstraction that is compatible with the network interface selected
     * @param technology The network technology that this server will communicate on.
     * @sa NetworkTypes::Technology
     * @param[in] protocol The protocol that the server should use.
     * @param error The error that occured while creating the server.
     * @attention While the IpServerAbstraction is useful for implementing this factory, it does not follow the L in SOLID for all derivations.
     *            The send and receive functions in the Http server use a different parameter type than the IpServerAbstraction
     *            so attempting to use an IpServerAbstraction as an HttpServer will not be possible. It's reccomended to wrap calls to the HttpServer
     *            in a function that dynamic_cast<>()'s the IpServerAbstraction to an HttpServer.
     * @sa https://en.wikipedia.org/wiki/SOLID
     */
    inline std::variant<IpServer, IpCellularServer, HttpServer> ServerFactory(NetworkTypes::Technology technology, IpFactory::Protocol protocol, ErrorType &error) {
        static_assert(IpServerFactoryRequirements<IpServer> && IpServerFactoryRequirements<IpCellularServer>, "Variant types must be derived from IpServerAbstraction");

        switch (technology) {
            case NetworkTypes::Technology::Wifi:
                switch (protocol) {
                    case IpFactory::Protocol::TcpOrUdp:
                        return std::variant<IpServer, IpCellularServer, HttpServer>(std::in_place_type<IpServer>);
                    case IpFactory::Protocol::Http:
                        return std::variant<IpServer, IpCellularServer, HttpServer>(std::in_place_type<HttpServer>);
                    case IpFactory::Protocol::Websocket:
                    case IpFactory::Protocol::Ftp:
                    case IpFactory::Protocol::Mqtt:
                    case IpFactory::Protocol::Modbus:
                    case IpFactory::Protocol::Unknown:
                    default: [[unlikely]]
                        assert(false);
                        return std::variant<IpServer, IpCellularServer, HttpServer>(std::in_place_type<IpServer>);
                }
            case NetworkTypes::Technology::Zigbee:
            case NetworkTypes::Technology::Ethernet:
            case NetworkTypes::Technology::Cellular:
                return std::variant<IpServer, IpCellularServer, HttpServer>(std::in_place_type<IpCellularServer>);
            case NetworkTypes::Technology::Unknown:
            default: [[unlikely]]
                assert(false);
                return std::variant<IpServer, IpCellularServer, HttpServer>(std::in_place_type<IpServer>);
        }
    }
}

#endif // __CELLULAR_IP_FACTORY_HPP__