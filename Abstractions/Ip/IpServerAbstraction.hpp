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
    IpServerAbstraction() : CommunicationProtocol() { _status.listening = false; }
    /// @brief Destructor
    virtual ~IpServerAbstraction() = default;

    /// @brief The tag for logging.
    static constexpr char TAG[] =  "IpServer";

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

    ///@brief Get a mutable reference to the socket
    Socket &getSocket() { return _socket; }
    ///@brief Get a constant reference to the socket
    const Socket &getSocketConst() const { return _socket; }
    ///@brief Get a mutable reference to the protocol
    IpServerSettings::Protocol &protocol() { return _protocol; }
    ///@brief Get a constant reference to the protocol
    const IpServerSettings::Protocol &protocolConst() const { return _protocol; }
    ///@brief Get a mutable reference to the version
    IpServerSettings::Version &version() { return _version; }
    ///@brief Get a constant reference to the version
    const IpServerSettings::Version &versionConst() const { return _version; }
    ///@brief Get a mutable reference to the port
    Port &port() { return _port; }
    ///@brief Get a constant reference to the port
    const Port &portConst() const { return _port; }
    /// @brief Get the network abstraction that this server communicates on as a mutable reference
    NetworkAbstraction &network() { assert(nullptr != _network); return *_network; }
    ///@brief Get the network abstraction as a constant reference
    const NetworkAbstraction &networkConst() const { assert(nullptr != _network); return *_network; }
    ///@brief Get a mutable reference to the status of the server
    ServerStatus &status() { return _status; }
    ///@brief Get a constant reference to the status of the server
    const ServerStatus &statusConst() const { return _status; }
    ///@brief Set the network abstraction
    ///@param[in] network The network abstraction to set
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