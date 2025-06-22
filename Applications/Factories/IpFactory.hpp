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
     * @param error
     * @sa ErrorType
     */
    inline ErrorType ClientFactory(NetworkTypes::Technology technology, ClientFactoryVariant &client) {
        ErrorType error = ErrorType::Success;

        switch (technology) {
            case NetworkTypes::Technology::Wifi:
            case NetworkTypes::Technology::Zigbee:
            case NetworkTypes::Technology::Ethernet:
                client.emplace<IpClient>();
                break;
            case NetworkTypes::Technology::Cellular:
                client.emplace<IpCellularClient>();
                break;
            case NetworkTypes::Technology::Unknown:
            default: [[unlikely]]
                return ErrorType::NotSupported;
        }

        return error;
    }
    /**
     * @brief This factory function creates an IpServerAbstraction that is compatible with the network interface selected
     * @param technology The network technology that this server will communicate on.
     * @sa NetworkTypes::Technology
     * @param error The error that occured while creating the server.
     * @attention While the IpServerAbstraction is useful for implementing this factory, it does not follow the L in SOLID for all derivations.
     *            The send and receive functions in the Http server use a different parameter type than the IpServerAbstraction
     *            so attempting to use an IpServerAbstraction as an HttpServer will not be possible. It's reccomended to wrap calls to the HttpServer
     *            in a function that dynamic_cast<>()'s the IpServerAbstraction to an HttpServer.
     * @sa https://en.wikipedia.org/wiki/SOLID
     */
    inline ErrorType ServerFactory(NetworkTypes::Technology technology, ServerFactoryVariant &server) {
        ErrorType error = ErrorType::Success;

        switch (technology) {
            case NetworkTypes::Technology::Wifi:
            case NetworkTypes::Technology::Ethernet:
            case NetworkTypes::Technology::Zigbee:
                server.emplace<IpServer>();
                break;
            case NetworkTypes::Technology::Cellular:
                server.emplace<IpCellularServer>();
                break;
            case NetworkTypes::Technology::Unknown:
            default: [[unlikely]]
                error = ErrorType::NotSupported;
        }

        return error;
    }
}

#endif // __CELLULAR_IP_FACTORY_HPP__