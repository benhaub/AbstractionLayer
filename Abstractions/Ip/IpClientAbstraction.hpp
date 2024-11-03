/***************************************************************************//**
* @author   Ben Haubrich
* @file     IpClientAbstraction.hpp
* @details  \b Synopsis: \n Interface for creating a client on any network
* @ingroup  AbstractionLayer
*******************************************************************************/
#ifndef __IP_CLIENT_ABSTRACTION_HPP__
#define __IP_CLIENT_ABSTRACTION_HPP__

//Foundation
#include "Types.hpp"
#include "Error.hpp"
//C++
#include <memory>
#include <string>
#include <functional>
#include <cassert>

/**
 * @struct ClientStatus
 * @brief The status of the client
*/
struct ClientStatus {
    bool connected; ///< True when the client is connected to the host.
};

/**
 * @namespace IpClientSettings
 * @brief Settings for the IP client
*/
namespace IpClientSettings {

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
 * @class IpClientAbstraction
 * @brief Abstraction for creating a client on any network
 * @note You should use the network to handle communication by placing events on it's queue.
*/
class IpClientAbstraction {

    public:
    /// @brief Constructor
    IpClientAbstraction() { _status.connected = false; }
    /// @brief Destructor
    virtual ~IpClientAbstraction() = default;

    /// @brief The tag for logging.
    static constexpr char TAG[] =  "IpClient";

    /**
     * @brief Connect to a host
     * @param[in] hostname The hostname to connect to
     * @param[in] port The port to connect to
     * @param[in] protocol The protocol to use
     * @sa IpSettings::Protocol
     * @param[in] version The version to use
     * @sa IpClientSettings::Version
     * @param[out] socket The socket that was created
     * @param[in] timeout The amount of time to wait to connect to the host
     * @post The caller is blocked until the connection is made or the timeout is reached. The connection can still be made after the timeout is reached.
     * @returns Fnd::ErrorType::Success on success
     * @returns Fnd::ErrorType::Failure on failure
     * @returns Fnd::ErrorType::NotImplemented if not implemented
     * @returns Fnd::ErrorType::NotSupported if the network interface doesn't support the operation
    */
    virtual ErrorType connectTo(const std::string &hostname, const Port port, const IpClientSettings::Protocol protocol, const IpClientSettings::Version version, Socket &socket, const Milliseconds timeout) = 0;
    /**
     * @brief Disconnect this client
     * @returns Fnd::ErrorType::Success on success
     * @returns Fnd::ErrorType::Failure on failure
     * @returns Fnd::ErrorType::NotImplemented if not implemented
    */
    virtual ErrorType disconnect() = 0;
    /**
     * @brief Send data to a socket
     * @param[in] data The data to send
     * @param[in] timeout The amount of time to wait to send the message
     * @pre data should be appropriately sized with the correct amount of bytes you want to send, i.e data.resize()
     * @returns Fnd::ErrorType::Success on success
     * @returns Fnd::ErrorType::Failure on failure
     * @returns Fnd::ErrorType::NotImplemented if not implemented
     * @returns Fnd::ErrorType::Timeout if a timeout occurred
    */
    virtual ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) = 0;
    /**
     * @brief Receive data from a socket
     * @pre buffer should be appropriately sized with the correct amount of bytes you want to send, i.e buffer.resize()
     * @param[out] buffer The buffer to receive data into
     * @param[in] timeout The amount of time to wait to receive the message
     * @returns ErrorType::Success on success
     * @returns ErrorType::Failure on failure
     * @returns ErrorType::NotImplemented if not implemented
     * @returns ErrorType::Timeout if a timeout occurred
     * @returns ErrorType::InvalidParameter if an invalid parameter was given
     * @post The number of bytes received is equal to the size of the buffer (i.e buffer.size())
    */
    virtual ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) = 0;
    /**
     * @brief Receive data from this client
     * @pre data should be appropriately sized with the correct amount of bytes you want to receive, i.e data.resize()
     * @param[in] data The data to send
     * @param[in] callback The callback to call when the data is received
     * @param[in] timeout The amount of time to wait to send the message.
     * @post If no callback is provided, then the function will wait for the timeout specified to occur before returning. If a callback is provided, then the function will return immediately.
     * @code
     * //TODO: add an example of a callback and how to give it to this function
     * @endcode
     * @returns ErrorType::Success on success
     * @returns ErrorType::Failure on failure
     * @returns ErrorType::NotImplemented if not implemented
     * @returns ErrorType::Timeout if a timeout occurred
     * @returns ErrorType::PrerequisitesNotMet if the buffer is not appropriately sized
    */
    virtual ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback = nullptr) = 0;
    /**
     * @brief Receive data from this client
     * @pre buffer should be appropriately sized with the correct amount of bytes you want to receive, i.e buffer.resize()
     * @param[out] buffer The buffer to receive data into
     * @param[in] timeout The amount of time to wait to receive the message. The message can still be received after the timeout is reached.
     * @param[in] callback The callback to call when the data is received
     * @post If no callback is provided, then the function will wait for the timeout specified to occur before returning. If a callback is provided, then the function will return immediately.
     * @code
     * //TODO: add an example of a callback and how to give it to this function
     * @endcode
     * @returns Fnd::ErrorType::Success on success
     * @returns Fnd::ErrorType::Failure on failure
     * @returns Fnd::ErrorType::NotImplemented if not implemented
     * @returns Fnd::ErrorType::Timeout if a timeout occurred
    */
    virtual ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback = nullptr) = 0;

    /// @brief Get the socket as a constant reference
    const Socket &sockConst() const { return _socket; }
    /// @brief Get the socket as a mutable reference
    Socket &sock() { return _socket; }
    /// @brief Get the protocol as a constant reference
    const IpClientSettings::Protocol &protocolConst() const { return _protocol; }
    /// @brief Get the protocol as a mutable reference
    IpClientSettings::Protocol &protocol() { return _protocol; }
    /// @brief Get the version as a constant reference
    const IpClientSettings::Version &versionConst() const { return _version; }
    /// @brief Get the version as a mutable reference
    IpClientSettings::Version &version() { return _version; }
    /// @brief Get the hostname as a constant reference
    const std::string &hostnameConst() const { return _hostname; }
    /// @brief Get the hostname as a mutable reference
    std::string &hostname() { return _hostname; }
    /// @brief Get the port as a constant reference
    const Port &portConst() const { return _port; }
    /// @brief Get the port as a mutable reference
    Port &port() { return _port; }
    /// @brief Get the network abstraction that this client communicates on as a mutable reference
    NetworkAbstraction &network() { assert(nullptr != _network); return *_network; }
    /// @brief Get the network abstraction that this client communicates on as a constant reference
    const NetworkAbstraction &networkConst() const { assert(nullptr != _network); return *_network; }
    /// @brief Get the status of the client as a const reference
    const ClientStatus &statusConst() const { return _status; }
    /// @brief Get the status of the client as a mutable reference
    ClientStatus &status() { return _status; }

    /// @brief Set the network abstraction that this client communicates on.
    /// @param network The network abstraction to use.
    ErrorType setNetwork(NetworkAbstraction &network) { _network = &network; return ErrorType::Success; }

    protected:
    /// @brief The socket
    Socket _socket = -1;
    /// @brief The protocol
    IpClientSettings::Protocol _protocol = IpClientSettings::Protocol::Unknown;
    /// @brief The IP version
    IpClientSettings::Version _version = IpClientSettings::Version::Unknown;
    /// @brief The hostname
    std::string _hostname = "";
    /// @brief The port
    Port _port = 0;
    /// @brief The status of the client
    ClientStatus _status;

    private:
    /// @brief The network interface that this client communicates on.
    /// @note Not a unique_ptr because this IP client does not have exclusive ownersip of the network
    NetworkAbstraction *_network = nullptr;
};

#endif // __IP_CLIENT_ABSTRACTION_HPP__