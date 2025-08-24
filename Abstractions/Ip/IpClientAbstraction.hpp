/***************************************************************************//**
* @author   Ben Haubrich
* @file     IpClientAbstraction.hpp
* @details  \b Synopsis: \n Interface for creating a client on any network
* @ingroup  Abstractions
*******************************************************************************/
#ifndef __IP_CLIENT_ABSTRACTION_HPP__
#define __IP_CLIENT_ABSTRACTION_HPP__

//AbstractionLayer
#include "NetworkAbstraction.hpp"
//C++
#include <functional>

/**
 * @namespace IpTypes
 * @brief Types for the IP client
*/
namespace IpClientTypes {

    /**
     * @struct Status
     * @brief The status of the client
    */
    struct Status {
        bool connected; ///< True when the client is connected to the host.
    };
}

/**
 * @class IpClientAbstraction
 * @brief Abstraction for creating a client on any network
 * @note You should use the network to handle communication by placing events on it's queue.
*/
class IpClientAbstraction {

    public:
    virtual ~IpClientAbstraction() = default;

    /// @brief The tag for logging.
    static constexpr char TAG[] =  "IpClient";

    /// @brief Print the status of the IP client
    void printStatus() const {
        PLT_LOGI(TAG, "<IpClientStatus> <Connected:%s> <Pie, Line>",
        statusConst().connected ? "true" : "false");
    }

    /**
     * @sa NetworkAbstraction::connectTo
    */
    virtual ErrorType connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout) = 0;
    /**
     * @sa NetworkAbstraction::disconnect
    */
    virtual ErrorType disconnect() = 0;
    /**
     * @brief Sends data.
     * @pre The amount of data to send is equal to the size of data. See std::string::resize()
     * @param[in] data The data to send.
     * @param[in] timeout The timeout in milliseconds.
     * @returns ErrorType::Success if the data was sent.
     * @returns ErrorType::Failure if the data was not sent.
     * @returns ErrorType::Timeout if the timeout was reached.
     * @note If the actual send is queued as an event for some other thread (i.e a network thread that actually sends the data)
     *       then the delay will be the timeout plus any additional scheduling delay incurred by the operating system as a result
     *       of thread priorities.
    */
    virtual ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) = 0;
    /**
     * @brief Receives data.
     * @param[out] buffer The data to receive.
     * @param[in] timeout The timeout in milliseconds.
     * @returns ErrorType::Success if the data was received.
     * @returns ErrorType::Failure if the data was not received.
     * @returns ErrorType::Timeout if the timeout was reached.
     * @returns ErrorType::NoData if the buffer has 0 length.
     * @post The amount of data received is equal to the size of the data if ErrorType::Success is returned. See std::string::size().
     * @note If the actual send is queued as an event for some other thread (i.e a network thread that actually sends the data)
     *       then the delay will be the timeout plus any additional scheduling delay incurred by the operating system as a result
     *       of thread priorities.
    */
    virtual ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) = 0;
    /**
     * @brief Sends data.
     * @pre The amount of data to send is equal to the size of data. See std::string::resize()
     * @param[in] data The data to send.
     * @param[in] timeout The time to wait to send the data.
     * @param[in] callback The callback to call when the data is sent.
     * @code{.cpp}
     * //Lambda callback
     * auto callback = [](const ErrorType error, const Bytes bytesWritten) -> void {
     *     if (ErrorType::Success == error) {
     *         // Data was sent
     *     }
     * };
     * error = sendNonBlocking(data, timeout, callback);
     * 
     * //Member function callback
     * void Foo::bar(const ErrorType error, const Bytes bytesWritten) {
     *     if (ErrorType::Success == error) {
     *         // Data was sent
     *     }
     * }
     * error = sendNonBlocking(data, timeout, std::bind(&Foo::bar, this, std::placeholders::_1, std::placeholders::_2)); 
     * @endcode
     * @returns ErrorType::Success if the data was sent.
     * @returns ErrorType::Failure if the data was not sent.
     * @post The callback will be called when the data has been sent. The bytes written is valid if and only if error is equal to ErrorType::Success.
    */
    virtual ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
        auto tx = [&, callback, data, timeout]() -> ErrorType {
            ErrorType error = ErrorType::Failure;

            assert(nullptr != callback);
            assert(nullptr != data.get());

            error = sendBlocking(*data, timeout);
            callback(error, data->size());

            return error;
        };

        EventQueue::Event event = EventQueue::Event(std::bind(tx));
        return network().addEvent(event);
    }
    /**
     * @brief Receives data.
     * @param[out] buffer The buffer to receive the data into.
     * @param[in] timeout The time to wait to receive the data.
     * @param[in] callback The callback to call when the data has been received.
     * @code{.cpp}
     * //Lambda callback
     * auto callback = [](const ErrorType error, std::shared_ptr<std::string> buffer) -> void {
     *     if (ErrorType::Success == error) {
     *         // Data was sent
     *     }
     * };
     * error = sendNonBlocking(data, timeout, callback);
     * 
     * //Member function callback
     * void Foo::bar(const ErrorType error, std::shared_ptr<std::string> buffer) {
     *     if (ErrorType::Success == error) {
     *         // Data was sent
     *     }
     * }
     * error = sendNonBlocking(data, timeout, std::bind(&Foo::bar, this, std::placeholders::_1, std::placeholders::_2)); 
     * @endcode
     * @post The callback will be called when the data has been received. The amount of data received is equal to the size of the
     *       data if ErrorType::Success is returned. See std::string::size().
    */
    virtual ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
        auto rx = [&, callback, buffer, timeout]() -> ErrorType {
            ErrorType error = ErrorType::Failure;

            assert(nullptr != callback);
            assert(nullptr != buffer.get());

            error = receiveBlocking(*buffer, timeout);
            callback(error, buffer);

            return error;
        };

        EventQueue::Event event = EventQueue::Event(std::bind(rx));
        return network().addEvent(event);
    }

    /// @brief Get the socket as a constant reference
    const Socket &sockConst() const { return _socket; }
    /// @brief Get the socket as a mutable reference
    Socket &sock() { return _socket; }
    /// @brief Get the protocol as a constant reference
    const IpTypes::Protocol &protocolConst() const { return _protocol; }
    /// @brief Get the protocol as a mutable reference
    IpTypes::Protocol &protocol() { return _protocol; }
    /// @brief Get the version as a constant reference
    const IpTypes::Version &versionConst() const { return _version; }
    /// @brief Get the version as a mutable reference
    IpTypes::Version &version() { return _version; }
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
    const IpClientTypes::Status &statusConst() const { return _status; }

    /// @brief Set the network abstraction that this client communicates on.
    /// @param[in] network The network abstraction to use.
    ErrorType setNetwork(NetworkAbstraction &network) { _network = &network; return ErrorType::Success; }

    protected:
    /// @brief The socket
    Socket _socket = -1;
    /// @brief The protocol
    IpTypes::Protocol _protocol = IpTypes::Protocol::Unknown;
    /// @brief The IP version
    IpTypes::Version _version = IpTypes::Version::Unknown;
    /// @brief The hostname
    std::string _hostname = "";
    /// @brief The port
    Port _port = 0;
    /// @brief The status of the client
    IpClientTypes::Status _status = {
        false
    };

    private:
    /// @brief The network interface that this client communicates on.
    /// @note Not a unique_ptr because this IP client does not have exclusive ownersip of the network
    NetworkAbstraction *_network = nullptr;
};

#endif // __IP_CLIENT_ABSTRACTION_HPP__