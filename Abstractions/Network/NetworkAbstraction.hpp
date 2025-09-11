/***************************************************************************//**
* @author   Ben Haubrich
* @file     NetworkAbstraction.hpp
* @details  \b Synopsis: \n Interface for communication over the network.
* @ingroup Abstractions
*******************************************************************************/
#ifndef __NETWORK_ABSTRACTION_HPP__
#define __NETWORK_ABSTRACTION_HPP__

//AbstractionLayer
#include "EventQueue.hpp"
#include "Log.hpp"
#include "Math.hpp"
#include "IpTypes.hpp"
//C++
#include <string>

/**
 * @namespace NetworkTypes
 * @brief Namespace for types related to the network interface.
 */
namespace NetworkTypes {
    
    /**
     * @enum Technology
     * @brief The technology of the network interface.
     * @note If you google wifi, 3g, or 4g, It will say they are all wireless communication technologies.
     *       That's where the name comes from.
    */
    enum class Technology {
        Unknown = 0, ///< Unknown
        Wifi,        ///< Wi-Fi
        Zigbee,      ///< ZigBee
        Ethernet,    ///< Ethernet
        Cellular,    ///< Cellular
    };

    /**
     * @struct Status
     * @brief The status of the network interface.
    */
    struct Status {
        bool isUp;             ///< True when the network is up and ready for use.
        Technology technology; ///< The technology of the network interface.

        Status() : isUp(false), technology(Technology::Unknown) {}
    };

    /// @brief Size of an IPv4 address string with tombstone
    constexpr uint8_t Ipv4AddressStringSize = 18;
    /// @brief Size of a MAC address string with tombstone
    constexpr uint8_t MacAddressStringSize = 18;
    /// @brief Size of a MAC address byte array
    constexpr uint8_t MacAddressByteArraySize = 6;

    /**
     * @brief Convert data from network byte order to host byte order.
     * @param[in] data The data to convert.
     * @returns The data in host byte order.
     */
    template<typename T>
    requires std::is_arithmetic_v<T>
    inline T NetworkToHostByteOrder(const T &data) {
        const uint16_t i = 1;

        //i is 0b00000001 in memory for big endian (network byte order)
        //i is 0b10000000 in memory for little endian
        //So reading as a byte, we'll either get 1 or 0.
        const bool hostIsLittleEndian = (*((char *)&i));
        if (hostIsLittleEndian) {
            T swappedData = 0;
            if (std::is_same<T, uint64_t>::value) {
                swappedData |= (data & 0x00000000000000FFULL) << ToBits(7);
                swappedData |= (data & 0x000000000000FF00ULL) << ToBits(5);
                swappedData |= (data & 0x0000000000FF0000ULL) << ToBits(3);
                swappedData |= (data & 0x00000000FF000000ULL) << ToBits(1);
                swappedData |= (data & 0x000000FF00000000ULL) >> ToBits(1);
                swappedData |= (data & 0x0000FF0000000000ULL) >> ToBits(3);
                swappedData |= (data & 0x00FF000000000000ULL) >> ToBits(5);
                swappedData |= (data & 0xFF00000000000000ULL) >> ToBits(7);
            }
            else if (std::is_same<T, uint32_t>::value) {
                swappedData |= (data & 0x000000FF) << ToBits(3);
                swappedData |= (data & 0x0000FF00) << ToBits(1);
                swappedData |= (data & 0x00FF0000) >> ToBits(1);
                swappedData |= (data & 0xFF000000) >> ToBits(3);
            }
            else if (std::is_same<T, uint16_t>::value) {
                swappedData |= (data & 0x00FF) << ToBits(1);
                swappedData |= (data & 0xFF00) >> ToBits(1);
            }

            return swappedData;
        }

        return data;
    }

    /**
     * @brief Convert data from host byte order to network byte order.
     * @param[in] data The data to convert.
     * @returns The data in network byte order.
     */
    template<typename T>
    inline T HostToNetworkByteOrder(const T &data) {
        return NetworkToHostByteOrder(data);
    }

    /**
     * @struct ConfigurationParameters 
     * @brief Contains the parameters used to configure the network.
     */
    struct ConfigurationParameters {
        public:
        /// @brief The technology type these parameters are meant for
        virtual NetworkTypes::Technology technology() const = 0;
    };
}

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
    virtual ErrorType transmit(const std::string &frame, const Socket socket, const Milliseconds timeout) = 0;
    /**
     * @brief Receive a frame of data.
     * @param[in] frameBuffer The buffer to store the received frame data.
     * @param[in] socket  The socket to receive from
     * @param[in] timeout The timeout in milliseconds to wait for the transmission to complete
     * @returns ErrorType::Success if the frame was successfully received
     * @returns ErrorType::Failure if the frame was not received
     * @post The frameBuffer is not modified in any way unless data is received.
     * @pre init
    */
    virtual ErrorType receive(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) = 0;
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
