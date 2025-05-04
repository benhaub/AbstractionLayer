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
#include "IpServerModule.hpp"
#include "HttpServerModule.hpp"
#include "Log.hpp"
#include "NetworkFactory.hpp"
//C++
#include <cassert>

///@brief The tag used for logging
static constexpr char IpFactoryTag[] = "IpFactoryTag";

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
    inline std::unique_ptr<IpClientAbstraction> ClientFactory(NetworkTypes::Technology technology, IpFactory::Protocol protocol, ErrorType &error) {
        switch (technology) {
            case NetworkTypes::Technology::Wifi: {
                auto client = std::make_unique<IpClient>();
                return client;
            }
            case NetworkTypes::Technology::Zigbee:
            case NetworkTypes::Technology::Ethernet:
            case NetworkTypes::Technology::Cellular:
            case NetworkTypes::Technology::Unknown:
            default: {
                error = ErrorType::NotSupported;
                break;
            }
        }

        return std::unique_ptr<IpClientAbstraction>(nullptr);
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
    inline std::unique_ptr<IpServerAbstraction> ServerFactory(NetworkTypes::Technology technology, IpFactory::Protocol protocol, ErrorType &error) {
        std::unique_ptr<IpServerAbstraction> server;

        switch (technology) {
            case NetworkTypes::Technology::Wifi:
                switch (protocol) {
                    case IpFactory::Protocol::TcpOrUdp:
                        server = std::make_unique<IpServer>();
                        return server;
                    case IpFactory::Protocol::Http:
                        server = std::make_unique<HttpServer>();
                        return server;
                    case IpFactory::Protocol::Websocket:
                    case IpFactory::Protocol::Ftp:
                    case IpFactory::Protocol::Mqtt:
                    case IpFactory::Protocol::Modbus:
                    case IpFactory::Protocol::Unknown:
                    default:
                        error = ErrorType::NotSupported;
                        break;
                }
            case NetworkTypes::Technology::Zigbee:
            case NetworkTypes::Technology::Ethernet:
            case NetworkTypes::Technology::Cellular:
            case NetworkTypes::Technology::Unknown:
            default:
                error = ErrorType::NotSupported;
                break;
        }

        return server;
    }
}

#endif // __CELLULAR_IP_FACTORY_HPP__