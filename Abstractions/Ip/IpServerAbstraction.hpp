/***************************************************************************//**
* @author   Ben Haubrich
* @file     IpServerAbstraction.hpp
* @details  \b Synopsis: \n Interface for creating a server on any network
* @ingroup  AbstractionLayer
*******************************************************************************/
#ifndef __IP_SERVER_ABSTRACTION_HPP__
#define __IP_SERVER_ABSTRACTION_HPP__

//AbstractionLayer
#include "CommunicationProtocol.hpp"
#include "Types.hpp"
#include "Error.hpp"
//C++
#include <memory>
#include <string>
#include <functional>

/**
 * @struct ServerStatus
 * @brief Server status
*/
struct ServerStatus {
    bool listening; ///< True when the server is listening for connections.
};

namespace IpServerSettings {

    /**
     * @enum Version
     * @brief The version to use for the IP connection
    */
    enum class Version : uint8_t {
        Unknown = 0, ///< Unknown
        IPv4,        ///< Internet Protocol Version 4
        IPv6         ///< Internet Protocol Version 6
    };

    /**
     * @enum Protocol
     * @brief The protocol to use for the IP connection
    */
    enum class Protocol : uint8_t {
        Unknown = 0, ///< Unknown
        Tcp,         ///< Transmission Control Protocol
        Udp          ///< User Datagram Protocol
    };
}

class NetworkAbstraction;


/**
 * @class IpServerAbstraction
 * @brief Creates a server on any network
 * @note You should use the network to handle communication by placing events on it's queue.
 */
class IpServerAbstraction : public CommunicationProtocol {

    public:
    /// @brief Constructor
    IpServerAbstraction() { _status.listening = false; };
    /// @brief Destructor
    virtual ~IpServerAbstraction() = default;

    /**
     * @brief Listen for connections on a port
     * @param[in] protocol The protocol to use for the connection
     * @sa IpServerSettings::Protocol
     * @param[in] version The version to use for the connection
     * @param[in] port The port to listen to
     * @sa IpServerSettings::Version
    */
    virtual ErrorType listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) = 0;
    /**
     * @brief Accept a connection from a client connecting to the socket given
     * @param[out] socket The socket that the connection was accepted on
     * @returns Fnd::ErrorType::Success on success
    */
    virtual ErrorType acceptConnection(Socket &socket) = 0;
    /**
     * @brief Close the connection
     * @returns Fnd::ErrorType::Success on success
    */
    virtual ErrorType closeConnection() = 0;

    /**
     * @brief Get the socket
    */
    Socket getSocket() const { return _socket; }
    /**
     * @brief Get the protocol
    */
    IpServerSettings::Protocol protocol() const { return _protocol; }
    /**
     * @brief Get the version
    */
    IpServerSettings::Version version() const { return _version; }
    /**
     * @brief Get the port
    */
    Port port() const { return _port; }
    /// @brief Get the network abstraction that this server communicates on as a mutable reference
    NetworkAbstraction &network() { assert(nullptr != _network); return *_network; }
    /**
     * @brief Get the network abstraction as a constant reference
    */
    const NetworkAbstraction &networkConst() const { return *_network; }
    /**
     * @brief Get the status of the server
    */
    ServerStatus status() const { return _status; }
    /**
     * @brief Set the network abstraction
    */
    ErrorType setNetwork(NetworkAbstraction &network) { _network = &network; return ErrorType::Success; }

    protected:
    /// @brief The socket
    Socket _socket = -1;
    /// @brief The protocol
    IpServerSettings::Protocol _protocol = IpServerSettings::Protocol::Unknown;
    /// @brief The IP version
    IpServerSettings::Version _version = IpServerSettings::Version::Unknown;
    /// @brief The port
    Port _port = 0;
    /// @brief The status of the server
    ServerStatus _status;

    private:
    /// @brief The network abstraction that this server communicates on.
    /// @note Not a unique_ptr because this IP server does not have exclusive ownersip of the network
    NetworkAbstraction *_network = nullptr;

};

#endif //__CBT_IP_SERVER_ABSTRACTION_HPP__