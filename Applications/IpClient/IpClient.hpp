/***************************************************************************//**
* @author   Ben Haubrich
* @file     IpClient.hpp
* @details  Interface for creating a client on any network
* @ingroup  Applications
*******************************************************************************/
#ifndef __IP_CLIENT_HPP__
#define __IP_CLIENT_HPP__

//AbstractionLayer
#include "NetworkAbstraction.hpp"
#include "OperatingSystemModule.hpp"
//C++
#include <memory>

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
 * @class IpClient
 * @brief Abstraction for creating a client on any network
 * @note You should use the network to handle communication by placing events on it's queue.
*/
class IpClient {

    public:
    virtual ~IpClient() = default;

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
    ErrorType connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout);
    /**
     * @sa NetworkAbstraction::disconnect
    */
    ErrorType disconnect();
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
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) {
        return sendBlockingImplementation(data, timeout);
    }
    /// @copydoc sendBlocking(const std::string &data, const Milliseconds timeout)
    ErrorType sendBlocking(const StaticString::Container &data, const Milliseconds timeout) {
        return sendBlockingImplementation(data, timeout);
    }
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
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) {
        return receiveBlockingImplementation(buffer, timeout);
    }
    /// @copydoc ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout)
    ErrorType receiveBlocking(StaticString::Container &buffer, const Milliseconds timeout) {
        return receiveBlockingImplementation(buffer,  timeout);
    }
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
    virtual ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback);
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
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback);

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

    /// @copydoc sendBlocking(const std::string &data, const Milliseconds timeout)
    template <typename Data>
    ErrorType sendBlockingImplementation(Data &data, const Milliseconds timeout) {
        bool doneSending = false;
        ErrorType callbackError = ErrorType::Failure;

        auto tx = [&]() -> ErrorType {
            callbackError = network().transmit(data, _socket, timeout);

            _status.connected = callbackError == ErrorType::Success;
            doneSending = true;
            return callbackError;
        };

        EventQueue::Event event = EventQueue::Event(tx);
        ErrorType error = network().addEvent(event);
        if (ErrorType::Success != error) {
            return error;
        }

        while (!doneSending) {
            OperatingSystem::Instance().delay(Milliseconds(1));
        }

        return callbackError;
    }
    /// @copydoc ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout)
    template <typename Buffer>
    ErrorType receiveBlockingImplementation(Buffer &buffer, const Milliseconds timeout) {
        bool doneReceiving = false;
        ErrorType callbackError = ErrorType::Failure;

        auto rx = [&]() -> ErrorType {

            callbackError = network().receive(buffer, _socket, timeout);

            _status.connected = callbackError == ErrorType::Success;
            doneReceiving = true;
            return callbackError;
        };

        EventQueue::Event event = EventQueue::Event(rx);
        ErrorType error = network().addEvent(event);
        if (ErrorType::Success != error) {
            return error;
        }

        while (!doneReceiving) {
            OperatingSystem::Instance().delay(Milliseconds(1));
        }

        return callbackError;
    }
};

#endif // __IP_CLIENT_ABSTRACTION_HPP__