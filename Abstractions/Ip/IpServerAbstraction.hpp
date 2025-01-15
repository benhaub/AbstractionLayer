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
    Count activeConnections;
};

/**
 * @namespace IpServerSettings
 * @brief Settings and confifguration for the Internet Protocol server.
 */
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
     * @param[in] timeout The time to wait to accept a connection.
     * @returns ErrorType::Success on success
     * @returns ErrorType::LimitReached if the maximum number of connections has been accepted
     * @returns ErrorType::Timeout if no connections were accepted within the given timeout.
     * @returns ErrorType::Failure otherwise
    */
    virtual ErrorType acceptConnection(Socket &socket, const Milliseconds timeout) = 0;
    /**
     * @brief Close the connection
     * @param[in] socket The socket to close
     * @returns Fnd::ErrorType::Success on success
    */
    virtual ErrorType closeConnection(const Socket socket) = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

    //Try casting to an IpServerAbstraction or IpServerModule if you are calling using a CommunicationProtocol pointer.
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override { return ErrorType::NotSupported; }
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override { return ErrorType::NotSupported; }
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override { return ErrorType::NotSupported; }
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override { return ErrorType::NotSupported; }

    //Send and receive functions include a socket since multiple sockets may be returned by accepting multiple connections.

    /**
     * @brief Sends data.
     * @param[in] data The data to send.
     * @param[in] timeout The timeout in milliseconds.
     * @param[in] socket The socket to send the data to.
     * @returns ErrorType::Success if the data was sent.
     * @returns ErrorType::Failure if the data was not sent.
     * @returns ErrorType::Timeout if the timeout was reached.
     * @post The amount of data to transmit is equal to the size of the data. See std::string::size(), std::string::resize().
    */
    virtual ErrorType sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket) = 0;
    /**
     * @brief Sends data.
     * @param[in] data The data to send.
     * @param[in] callback The callback to call when the data is sent.
     * @param[in] socket The socket to send the data to.
     * @returns ErrorType::Success if the data was sent.
     * @returns ErrorType::Failure if the data was not sent.
     * @post The callback will be called when the data has been sent. The bytes written is valid if and only if error is equal to ErrorType::Success.
    */
    virtual ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) = 0;
    /**
     * @brief Receives data.
     * @param[in] buffer The data to receive.
     * @param[in] timeout The timeout in milliseconds.
     * @param[out] socket The socket in which the data was received from.
     * @returns ErrorType::Success if the data was received.
     * @returns ErrorType::Failure if the data was not received.
     * @returns ErrorType::Timeout if the timeout was reached.
     * @post The amount of data received is equal to the size of the data. See std::string::size(), std::string::resize().
    */
    virtual ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket) = 0;
    /**
     * @brief Receives data.
     * @param[in] buffer The buffer to receive the data into.
     * @param[in] callback The callback to call when the data has been received.
     * @post The callback will be called when the data has been received. The buffer is valid if and only if error is equal to ErrorType::Success.
    */
    virtual ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) = 0;

#pragma GCC diagnostic pop

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
    /// @brief The socket on which we listen for new connections
    Socket _listenerSocket = -1;
    /// @brief The protocol
    IpServerSettings::Protocol _protocol = IpServerSettings::Protocol::Unknown;
    /// @brief The IP version
    IpServerSettings::Version _version = IpServerSettings::Version::Unknown;
    /// @brief The port
    Port _port = 0;
    /// @brief The status of the server
    ServerStatus _status;
    /// @brief list of all the sockets we have accepted connection for
    std::vector<Socket> _connectedSockets;

    private:
    /// @brief The network abstraction that this server communicates on.
    /// @note Not a unique_ptr because this IP server does not have exclusive ownersip of the network
    NetworkAbstraction *_network = nullptr;

};

#endif //__CBT_IP_SERVER_ABSTRACTION_HPP__