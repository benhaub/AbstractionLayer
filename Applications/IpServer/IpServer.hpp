/***************************************************************************//**
* @author  Ben Haubrich
* @file    IpServer.hpp
* @details Interface for creating a server on any network
* @ingroup Abstractions
*******************************************************************************/
#ifndef __IP_SERVER_HPP__
#define __IP_SERVER_HPP__

//AbstractionLayer
#include "NetworkAbstraction.hpp"
#include "OperatingSystemModule.hpp"
//C++
#include <memory>

/**
 * @namespace IpTypes
 * @brief Types related to the Internet Protocol server.
 */
namespace IpServerTypes {

    /**
     * @struct Status
     * @brief Server status
    */
    struct Status {
        bool listening;          ///< True when the server is listening for connections.
        Count activeConnections; ///< The number of simultaneous active connections.

        Status() : listening(false), activeConnections(0) {}
    };
}

/**
 * @class IpServer
 * @brief Creates a server on any network
 * @note You should use the network to handle communication by placing events on it's queue.
 */
class IpServer {

    public:
    /// @brief Destructor
    virtual ~IpServer() = default;

    /// @brief The tag for logging.
    static constexpr char TAG[] =  "IpServer";
    /// @brief Print the status if the IP server.
    void printStatus() {
        PLT_LOGI(TAG, "<IpServerStatus> <Listening:%s, Active Connections:%u> <Pie, Line>",
            status().listening ? "true" : "false", status().activeConnections);
    }

    /**
     * @sa NetworkAbstraction::listenTo
    */
    ErrorType listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port);
    /**
     * @sa NetworkAbstraction::acceptConnection
    */
    ErrorType acceptConnection(Socket &socket, const Milliseconds timeout);
    /**
     * @sa NetworkAbstraction::closeConnection
    */
    ErrorType closeConnection(const Socket socket);
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
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket) {
        return sendBlockingImplementation(data, timeout, socket);
    }
    /// @copydoc sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket)
    ErrorType sendBlocking(const StaticString::Container &data, const Milliseconds timeout, const Socket socket) {
        return sendBlockingImplementation(data, timeout, socket);
    }
    /**
     * @brief Receives data.
     * @param[in] buffer The data to receive.
     * @param[in] timeout The timeout in milliseconds.
     * @param[out] socket If negative, will contain the socket in which the data was received from. Otherwise it will be the socket in which data is explicitely received from.
     * @returns ErrorType::Success if the data was received.
     * @returns ErrorType::Failure if the data was not received.
     * @returns ErrorType::Timeout if the timeout was reached.
     * @post The amount of data received is equal to the size of the data. See std::string::size(), std::string::resize().
    */
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket) {
        return receiveBlockingImplementation(buffer, timeout, socket);
    }
    /// @copydoc receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket)
    ErrorType receiveBlocking(StaticString::Container &buffer, const Milliseconds timeout, Socket &socket) {
        return receiveBlockingImplementation(buffer, timeout, socket);
    }
    /**
     * @brief Sends data.
     * @param[in] data The data to send.
     * @param[in] timeout The time to wait for the data to be sent
     * @param[in] socket The socket to send the data to.
     * @param[in] callback The callback to call when the data has been sent.
     * @returns ErrorType::Success if the data was sent.
     * @returns ErrorType::Failure if the data was not sent.
     * @post The callback will be called when the data has been sent. The bytes written is valid if and only if error is equal to ErrorType::Success.
    */
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback);
    /**
     * @brief Receives data.
     * @param[in] buffer The buffer to receive the data into.
     * @param[in] timeout The time to wait to receive the data.
     * @param[in] callback The callback to call when the data has been received.
     * @returns ErrorType::Success if the data was received.
     * @returns ErrorType::Failure if the data was not received.
     * @returns ErrorType::Timeout if the timeout was reached.
     * @post The callback will be called when the data has been received. The buffer is valid if and only if error is equal to ErrorType::Success.
    */
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback);

    ///@brief Get a mutable reference to the protocol
    IpTypes::Protocol &protocol() { return _protocol; }
    ///@brief Get a constant reference to the protocol
    const IpTypes::Protocol &protocolConst() const { return _protocol; }
    ///@brief Get a mutable reference to the version
    IpTypes::Version &version() { return _version; }
    ///@brief Get a constant reference to the version
    const IpTypes::Version &versionConst() const { return _version; }
    ///@brief Get a mutable reference to the port
    Port &port() { return _port; }
    ///@brief Get a constant reference to the port
    const Port &portConst() const { return _port; }
    /// @brief Get the network abstraction that this server communicates on as a mutable reference
    NetworkAbstraction &network() { assert(nullptr != _network); return *_network; }
    ///@brief Get the network abstraction as a constant reference
    const NetworkAbstraction &networkConst() const { assert(nullptr != _network); return *_network; }
    ///@brief Set the network abstraction
    ///@param[in] network The network abstraction to set
    void setNetwork(NetworkAbstraction &network) { _network = &network; }
    ///@brief Get a constant reference to the status of the server
    const IpServerTypes::Status &status() const {
        return _status;
    }

    protected:
    /// @brief The socket on which we listen for new connections
    Socket _listenerSocket = -1;
    /// @brief The protocol
    IpTypes::Protocol _protocol = IpTypes::Protocol::Unknown;
    /// @brief The IP version
    IpTypes::Version _version = IpTypes::Version::Unknown;
    /// @brief The port
    Port _port = 0;
    /// @brief The status of the server
    IpServerTypes::Status _status;
    /// @brief list of all the sockets we have accepted connection for
    std::vector<Socket> _connectedSockets = {};

    private:
    /// @brief The network abstraction that this server communicates on.
    /// @note Not a unique_ptr because this IP server does not have exclusive ownersip of the network
    NetworkAbstraction *_network = nullptr;

    /// @copydoc sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket)
    template <typename Data>
    ErrorType sendBlockingImplementation(Data &data, const Milliseconds timeout, const Socket socket) {
        bool sent = false;
        ErrorType callbackError = ErrorType::Failure;
        Id thread;
        OperatingSystem::Instance().currentThreadId(thread);

        auto tx = [&, thread]() -> ErrorType {
            callbackError = network().transmit(data, socket, timeout);

            sent = true;
            OperatingSystem::Instance().unblock(thread);
            return callbackError;
        };

        EventQueue::Event event = EventQueue::Event(tx);
        ErrorType error = network().addEvent(event);
        if (ErrorType::Success != error) {
            return error;
        }

        while (!sent) {
            OperatingSystem::Instance().block();
        }

        return callbackError;
    }
    /// @copydoc receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket)
    template <typename Buffer>
    ErrorType receiveBlockingImplementation(Buffer &buffer, const Milliseconds timeout, Socket &socket) {
        bool received = false;
        ErrorType callbackError = ErrorType::NoData;
        Id thread;
        OperatingSystem::Instance().currentThreadId(thread);

        auto rx = [&, thread]() -> ErrorType {
            if (-1 == socket) {
                for (size_t i = 0; i < _connectedSockets.size(); i++) {
                    callbackError = network().receive(buffer, _connectedSockets[i], timeout);
                    socket = _connectedSockets[i];
                    if (ErrorType::Success == callbackError) {
                        break;
                    }
                }
            }
            else {
                callbackError = network().receive(buffer, socket, timeout);
            }

            received = true;
            OperatingSystem::Instance().unblock(thread);
            return callbackError;
        };

        EventQueue::Event event = EventQueue::Event(rx);
        ErrorType error = network().addEvent(event);
        if (ErrorType::Success != error) {
            return error;
        }

        while (!received) {
            OperatingSystem::Instance().block();
        }

        return callbackError;
    }
};

#endif //__IP_SERVER_HPP__
