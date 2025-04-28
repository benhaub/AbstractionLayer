/***************************************************************************//**
* @author   Ben Haubrich
* @file     IcCommunicationProtocol.hpp
* @details  \b Synopsis: \n Interface for integrated circuit communication
* @ingroup Abstractions
*******************************************************************************/
#ifndef __IC_COMMUNICATION_PROTOCOL_HPP__
#define __IC_COMMUNICATION_PROTOCOL_HPP__

//AbstractionLayer
#include "EventQueue.hpp"
//C++
#include <memory>

namespace IcCommunicationProtocolTypes {
    /**
     * @enum IcDevice
     * @brief Enumeration for different types of IC devices.
    */
    enum class IcDevice : uint8_t {
        Unknown = 0, ///< Unknown peripheral
        Uart,        ///< UART peripheral
        Spi,         ///< SPI peripheral
        I2c,         ///< I2C peripheral
        Ethernet,    ///< Ethernet peripheral
        Usb,         ///< USB peripheral
    };
}

/**
 * @class IcCommunicationProtocol
 * @brief Interface for integrated circuit communication
*/
class IcCommunicationProtocol : public EventQueue {

    public:
    ///@brief constructor
    IcCommunicationProtocol() : EventQueue() {}
    ///@brief destructor
    virtual ~IcCommunicationProtocol() = default;

    /**
     * @brief Initialize the IC device
     * @pre You may need to call configuration functions before calling this function
     * @returns ErrorType::Success if the device was initialized successfully
     * @returns ErrorType::Failure otherwise
     * @post The device is read to use after this call.
     */
    virtual ErrorType init() = 0;
    /**
     * @brief Deinitialize the IC device
     * @returns ErrorType::Success if the device was deinitialized successfully
     * @returns ErrorType::Failure otherwise
     */
    virtual ErrorType deinit() = 0;
    /**
     * @brief transmit data
     * @sa Fnd::CommunicationProtocol::sendBlocking
    */
    virtual ErrorType txBlocking(const std::string &data, const Milliseconds timeout) = 0;
    /**
     * @brief transmit data
     * @sa Fnd::CommunicationProtocol::sendNonBlocking
    */
    virtual ErrorType txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) = 0;
    /**
     * @brief receive data
     * @sa Fnd::CommunicationProtocol::receiveBlocking
    */
    virtual ErrorType rxBlocking(std::string &buffer, const Milliseconds timeout) = 0;
    /**
     * @brief receive data
     * @sa Fnd::CommunicationProtocol::receiveNonBlocking
    */
    virtual ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) = 0;
    /**
     * @brief flush the receive buffer
    */
    virtual ErrorType flushRxBuffer() = 0;
};

#endif //__IC_COMMUNICATION_PROTOCOL_HPP__
