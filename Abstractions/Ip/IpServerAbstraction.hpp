/***************************************************************************//**
* @author   Ben Haubrich
* @file     IpServerAbstraction.hpp
* @details  \b Synopsis: \n Interface for creating a server on any network
* @ingroup Abstractions
*******************************************************************************/
#ifndef __IP_SERVER_ABSTRACTION_HPP__
#define __IP_SERVER_ABSTRACTION_HPP__

//AbstractionLayer
#include "Log.hpp"
#include "IpTypes.hpp"
#include "EventQueue.hpp"
#include "NetworkAbstraction.hpp"

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
    };
}


/**
 * @class IpServerAbstraction
 * @brief Creates a server on any network
 * @note You should use the network to handle communication by placing events on it's queue.
 */
class IpServerAbstraction : public EventQueue {

    public:
    /// @brief Constructor
    IpServerAbstraction() : EventQueue() {}
    /// @brief Destructor
    virtual ~IpServerAbstraction() = default;

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
    virtual ErrorType listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) = 0;
    /**
     * @sa NetworkAbstraction::acceptConnection
    */
    virtual ErrorType acceptConnection(Socket &socket, const Milliseconds timeout) = 0;
    /**
     * @sa NetworkAbstraction::closeConnection
    */
    virtual ErrorType closeConnection(const Socket socket) = 0;
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
     * @brief Receives data.
     * @param[in] buffer The data to receive.
     * @param[in] timeout The timeout in milliseconds.
     * @param[out] socket If negative, will contain the socket in which the data was received from. Otherwise it will be the socket in which data is explicitely received from.
     * @returns ErrorType::Success if the data was received.
     * @returns ErrorType::Failure if the data was not received.
     * @returns ErrorType::Timeout if the timeout was reached.
     * @post The amount of data received is equal to the size of the data. See std::string::size(), std::string::resize().
    */
    virtual ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket) = 0;
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
    virtual ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {

        auto tx = [&, callback, data, timeout, socket]() -> ErrorType {
            ErrorType error = ErrorType::Failure;

            assert(nullptr != callback);

            if (nullptr == data.get()) {
                error = ErrorType::NoData;
            }
            else {
                error = sendBlocking(*data, timeout, socket);
            }

            callback(error, data->size());
            return error;
        };

        EventQueue::Event event = EventQueue::Event(std::bind(tx));
        return network().addEvent(event);
    }
    /**
     * @brief Receives data.
     * @param[in] buffer The buffer to receive the data into.
     * @param[in] timeout The time to wait to receive the data.
     * @param[out] socket If negative, will contain the socket in which the data was received from. Otherwise it will be the socket in which data is explicitely received from.
     * @param[in] callback The callback to call when the data has been received.
     * @returns ErrorType::Success if the data was received.
     * @returns ErrorType::Failure if the data was not received.
     * @returns ErrorType::Timeout if the timeout was reached.
     * @post The callback will be called when the data has been received. The buffer is valid if and only if error is equal to ErrorType::Success.
    */
    virtual ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) {
        auto receiveCallback = [&, callback, buffer, timeout]() -> ErrorType {
            ErrorType error = ErrorType::Failure;
            Socket socket = -1;

            assert(nullptr != callback);

            if (nullptr == buffer.get()) {
                error = ErrorType::NoData;
                callback(error, socket, buffer);
                return error;
            }

            error = receiveBlocking(*buffer, timeout, socket);

            callback(error, socket, buffer);

            return error;
        };

        EventQueue::Event event = EventQueue::Event(receiveCallback);
        return network().addEvent(event);
    }

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
    const IpServerTypes::Status &status() {
        _status.activeConnections = _connectedSockets.size();
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
    IpServerTypes::Status _status = {
        false,
        0
    };
    /// @brief list of all the sockets we have accepted connection for
    std::vector<Socket> _connectedSockets;

    private:
    /// @brief The network abstraction that this server communicates on.
    /// @note Not a unique_ptr because this IP server does not have exclusive ownersip of the network
    NetworkAbstraction *_network = nullptr;

};

#endif //__IP_SERVER_ABSTRACTION_HPP__
