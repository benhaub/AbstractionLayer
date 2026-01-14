/***************************************************************************//**
* @author  Ben Haubrich
* @file    NetworkAbstraction.hpp
* @details Interface for communication over the network.
* @ingroup Abstractions
*******************************************************************************/
#ifndef __NETWORK_ABSTRACTION_HPP__
#define __NETWORK_ABSTRACTION_HPP__

//AbstractionLayer
#include "EventQueue.hpp"
#include "Log.hpp"
#include "StaticString.hpp"
#include "NetworkTypes.hpp"
//C++
#include <string>

/**
 * @class NetworkAbstraction
 * @brief Interface for communication over the network.
*/
class NetworkAbstraction : public EventQueue {

    public:
    /// @brief Default destructor
    virtual ~NetworkAbstraction() = default;

    /// @brief Tag for logging
    static constexpr char TAG[] = "Network";

    /**
     * @brief Configure the network before initializing
     * @param[in] parameters The parameters to configure with
     * @sa ConfigurationParameters
     */
    virtual ErrorType configure(const NetworkTypes::ConfigurationParameters &parameters) = 0;
    /**
    * @brief Initialize the interface.
    * @pre Call configure first.
    * @returns ErrorType::Success if the network interface was initialized.
    * @returns ErrorType::Failure otherwise
    * @pre configure
    */
    virtual ErrorType init() = 0;
    /**
     * @brief Bring up the network interface so that it is ready for use (e.g. IP connections)
     * @returns ErrorType::Success if the network interface was brought up successfully
     * @returns ErrorType::Failure if the network interface could not be brought up
     * @pre init
    */
    virtual ErrorType networkUp() = 0;
    /**
     * @brief Bring down the network interface.
     * @returns ErrorType::Success if the network interface was brought down successfully
     * @returns ErrorType::Failure if the network interface could not be brought down
     * @pre init
    */
    virtual ErrorType networkDown() = 0;
    /**
     * @brief Connect to a host
     * @param[in] hostname The hostname to connect to
     * @param[in] port The port to connect to
     * @param[in] protocol The protocol to use
     * @sa IpSettings::Protocol
     * @param[in] version The version to use
     * @sa IpTypes::Version
     * @param[out] sock The socket that was created
     * @param[in] timeout The amount of time to wait to connect to the host
     * @post The caller is blocked until the connection is made or the timeout is reached. The connection can still be made after the timeout is reached.
     * @returns Fnd::ErrorType::Success on success
     * @returns Fnd::ErrorType::Failure on failure
     * @returns Fnd::ErrorType::NotImplemented if not implemented
     * @returns Fnd::ErrorType::NotSupported if the network interface doesn't support the operation
     * @pre init
    */
    virtual ErrorType connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &sock, const Milliseconds timeout) = 0;
    /**
     * @brief Disconnect this client
     * @param[in] socket The socket to disconnect
     * @returns Fnd::ErrorType::Success on success
     * @returns Fnd::ErrorType::Failure on failure
     * @returns Fnd::ErrorType::NotImplemented if not implemented
     * @pre init
    */
    virtual ErrorType disconnect(const Socket &socket) = 0;
    /**
     * @brief Listen for connections on a port
     * @param[in] protocol The protocol to use for the connection
     * @sa IpTypes::Protocol
     * @param[in] version The version to use for the connection
     * @param[in] port The port to listen to
     * @sa IpTypes::Version
     * @param[out] listenerSocket The socket that was created to listen for connections
     * @pre init
    */
    virtual ErrorType listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port, Socket &listenerSocket) = 0;
    /**
     * @brief Accept a connection from a client connecting to the socket given
     * @param[in] listenerSocket The socket to accept connections on that was returned by listenTo
     * @param[out] newSocket The socket that the connection was accepted on
     * @param[in] timeout The time to wait to accept a connection.
     * @returns ErrorType::Success on success
     * @returns ErrorType::LimitReached if the maximum number of connections has been accepted
     * @returns ErrorType::Timeout if no connections were accepted within the given timeout.
     * @returns ErrorType::Failure otherwise
     * @pre init
    */
    virtual ErrorType acceptConnection(const Socket &listenerSocket, Socket &newSocket, const Milliseconds timeout) = 0;
    /**
     * @brief Close the connection
     * @param[in] socket The socket to close
     * @returns Fnd::ErrorType::Success on success
     * @pre init
    */
    virtual ErrorType closeConnection(const Socket socket) = 0;
    /**
     * @brief Transmit a frame of data.
     * @param[in] frame The frame of data to transmit
     * @param[in] socket The socket to transmit from
     * @param[in] timeout The timeout in milliseconds to wait for the transmission to complete
     * @returns ErrorType::Success if the transmission was successful
     * @returns ErrorType::Failure if the transmission failed
     * @post NetworkTypes::Status::isUp will be set to false after this function returns ErrorType::Success
     * @pre init
    */
    virtual ErrorType transmit(const std::string &frame, const Socket socket, const Milliseconds timeout) {
        return transmit(std::string_view(frame.data(), frame.size()), socket, timeout);
    }
    /// @copydoc ErrorType transmit(const std::string &frame, const Socket socket, const Milliseconds timeout)
    virtual ErrorType transmit(const StaticString::Container &frame, const Socket socket, const Milliseconds timeout) {
        return transmit(std::string_view(frame->c_str(), frame->size()), socket, timeout);
    }
    /// @copydoc ErrorType transmit(const std::string &frame, const Socket socket, const Milliseconds timeout)
    virtual ErrorType transmit(std::string_view frame, const Socket socket, const Milliseconds timeout) = 0;
    /**
     * @brief Receive a frame of data less than or equal to the buffer size.
     * @param[in] frameBuffer The buffer to store the received frame data.
     * @param[in] socket  The socket to receive from
     * @param[in] timeout The timeout in milliseconds to wait for the transmission to complete
     * @returns ErrorType::Success if the frame was successfully received
     * @returns ErrorType::Failure if the frame was not received
     * @returns ErrorType::NoData if no data was received. 0 == frameBuffer.size()
     * @post The frameBuffer is not modified in any way unless data is received.
     * @pre init
    */
    virtual ErrorType receive(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) {
        Bytes read = 0;
        const ErrorType error = receive(&frameBuffer[0], frameBuffer.size(), socket, read, timeout);

        if (ErrorType::Success == error) {
            frameBuffer.resize(read);
        }

        return error;
    }
    /// @copydoc ErrorType receive(std::string &frameBuffer, const Socket socket, const Milliseconds timeout)
    virtual ErrorType receive(StaticString::Container &frameBuffer, const Socket socket, const Milliseconds timeout) {
        Bytes read = 0;
        const ErrorType error = receive(&frameBuffer[0], frameBuffer->size(), socket, read, timeout);

        if (ErrorType::Success == error) {
            frameBuffer->resize(read);
        }

        return error;
    }
    /// @copydoc ErrorType receive(std::string &frameBuffer, const Socket socket, const Milliseconds timeout)
    virtual ErrorType receive(char *frameBuffer, const size_t bufferSize, const Socket socket, Bytes &read, const Milliseconds timeout) = 0;
    /**
     * @brief Get the MAC address of this network interface.
     * @param[out] macAddress The MAC address of this network interface.
     * @returns ErrorType::Success if the MAC address was successfully retrieved
     * @returns ErrorType::Failure if the MAC address was not successfully retrieved
     * @pre init
    */
    virtual ErrorType getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) = 0;
    /**
    * @brief Get the signal strength of the network interface.
    * @param[out] signalStrength The signal strength of the network interface.
    * @returns ErrorType::Success if the signal strength was successfully retrieved
    * @returns ErrorType::Negative if the signal strength is not known or not detectable.
    * @returns ErrorType::Failure if the signal strength was not successfully retrieved
    * @pre init
    */
    virtual ErrorType getSignalStrength(DecibelMilliWatts &signalStrength) = 0;

    /// @brief The current status of the network interface as a const reference.
    const NetworkTypes::Status &status() const {
        return _status;
    }

    protected:
    /// @brief The current status of the network interface
    NetworkTypes::Status _status;
};

#endif // __NETWORK_ABSTRACTION_HPP__