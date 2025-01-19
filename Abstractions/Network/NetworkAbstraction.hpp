/***************************************************************************//**
* @author   Ben Haubrich
* @file     NetworkAbstraction.hpp
* @details  \b Synopsis: \n Interface for communication over the network.
* @ingroup Abstractions
*******************************************************************************/
#ifndef __NETWORK_ABSTRACTION_HPP__
#define __NETWORK_ABSTRACTION_HPP__

//Foundation
#include "EventQueue.hpp"
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
        bool isUp;                           ///< True when the network is up and ready for use.
        Technology technology;               ///< The technology of the network interface.
        std::string manufacturerName;        ///< The manufacturer name of the network interface.
        DecibelMilliWatts signalStrength;    ///< The signal strength of the network interface.
    };
}

/**
 * @class NetworkAbstraction
 * @brief Interface for communication over the network.
*/
class NetworkAbstraction : public EventQueue {

    public:
    /// @brief Default constructor
    NetworkAbstraction() = default;
    /// @brief Default destructor
    virtual ~NetworkAbstraction() = default;

    /// @brief Tag for logging
    static constexpr char TAG[] = "Network";

    /**
    * @brief Initialize the interface.
    * @returns ErrorType::Success if the network interface was initialized and ready for clients to connect.
    * @returns ErrorType::Failure if the network interface was not initialzed and is not ready for clients to connect.
    * @post May block for up to a maximum of 10 seconds to bring the interface up.
    * @post Network can be used to connect after this function returns ErrorType::Success.
    * @post Will init with a default setting if network parameters are not set prior to this call.
    */
    virtual ErrorType init() = 0;
    /**
     * @brief Bring up the network interface so that it is ready for use (e.g. IP connections)
     * @returns ErrorType::Success if the network interface was brought up successfully
     * @returns ErrorType::Failure if the network interface could not be brought up
    */
    virtual ErrorType networkUp() = 0;
    /**
     * @brief Bring down the network interface.
     * @returns ErrorType::Success if the network interface was brought down successfully
     * @returns ErrorType::Failure if the network interface could not be brought down
    */
    virtual ErrorType networkDown() = 0;
    /**
     * @brief Transmit a frame of data.
     * @param frame The frame of data to transmit
     * @param timeout The timeout in milliseconds to wait for the transmission to complete
     * @returns ErrorType::Success if the transmission was successful
     * @returns ErrorType::Failure if the transmission failed
    */
    virtual ErrorType txBlocking(const std::string &frame, const Socket socket, const Milliseconds timeout) = 0;
    /**
     * @sa txBlocking
     * @param frame The frame to transmit.
     * @param callback Function that is called when transmission is complete
     * @code 
     * //Function member signature:
     * void callback(ErrorType error, const Bytes bytesWritten) { return ErrorType::Success; }
     * //lambda signature:
     * auto callback = [](const ErrorType error, const Bytes bytesWritten) { return ErrorType::Success; }
     * @endcode
    */
    virtual ErrorType txNonBlocking(const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) = 0;
    /**
     * @brief Receive a frame of data.
     * @param frameBuffer The buffer to store the received frame data.
     * @param timeout The timeout in milliseconds to wait for the transmission to complete
     * @returns ErrorType::Success if the frame was successfully received
     * @returns ErrorType::Failure if the frame was not received
    */
    virtual ErrorType rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) = 0;
    /**
     * @sa rxBlocking
     * @param frameBuffer The buffer to store the received frame data.
     * @param callback Function that is called when the frame has been received
     * @code 
     * //Function member signature:
     * void callback(ErrorType error, std::shared_ptr<std::string> frameBuffer) { return ErrorType::Success; }
     * //lambda signature:
     * auto callback = [](const ErrorType error, std::shared_ptr<std::string> frameBuffer) { return ErrorType::Success; }
     * @endcode
    */
    virtual ErrorType rxNonBlocking(std::shared_ptr<std::string> frameBuffer, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> frameBuffer)> callback) = 0;
    /**
     * @brief Get the MAC address of this network interface.
     * @param[out] macAddress The MAC address of this network interface.
     * @returns ErrorType::Success if the MAC address was successfully retrieved
     * @returns ErrorType::Failure if the MAC address was not successfully retrieved
    */
    virtual ErrorType getMacAddress(std::string &macAddress) = 0;
    /**
    * @brief Get the signal strength of the network interface.
    * @param[out] signalStrength The signal strength of the network interface.
    * @returns ErrorType::Success if the signal strength was successfully retrieved
    * @returns ErrorType::Failure if the signal strength was not successfully retrieved
    */
    virtual ErrorType getSignalStrength(DecibelMilliWatts &signalStrength) = 0;

    /// @brief The current status of the network interface as a const reference.
    const NetworkTypes::Status &statusConst() { return _status; }

    protected:
    /// @brief The current status of the network interface
    NetworkTypes::Status _status;
};

#endif // __NETWORK_ABSTRACTION_HPP__