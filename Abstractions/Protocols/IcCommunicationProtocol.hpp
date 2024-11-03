/***************************************************************************//**
* @author   Ben Haubrich
* @file     IcCommunicationProtocol.hpp
* @details  \b Synopsis: \n Interface for integrated circuit communication
* @ingroup  AbstractionLayer
*******************************************************************************/
#ifndef __IC_COMMUNICATION_PROTOCOL_HPP__
#define __IC_COMMUNICATION_PROTOCOL_HPP__

//Foundation
#include "CommunicationProtocol.hpp"

namespace IcCommunicationProtocolTypes {
    /**
     * @enum CellularIcDevice
     * @brief Enumeration for different types of Cellular IC devices.
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
class IcCommunicationProtocol : public CommunicationProtocol {

    public:
    /**
     * @brief constructor
    */
    IcCommunicationProtocol() : CommunicationProtocol() {}
    /**
     * @brief destructor
    */
    virtual ~IcCommunicationProtocol() = default;

    /**
     * @brief transmit data
     * @sa Fnd::CommunicationProtocol::send
    */
    virtual ErrorType txBlocking(const std::string &data, const Milliseconds timeout) = 0;
    /**
     * @brief transmit data
     * @sa Fnd::CommunicationProtocol::sendNonBlocking
    */
    virtual ErrorType txNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback = nullptr) = 0;
    /**
     * @brief receive data
     * @sa Fnd::CommunicationProtocol::receive
    */
    virtual ErrorType rxBlocking(std::string &buffer, const Milliseconds timeout) = 0;
    /**
     * @brief receive data
     * @sa Fnd::CommunicationProtocol::receiveNonBlocking
    */
    virtual ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback = nullptr) = 0;
    /**
     * @brief flush the receive buffer
    */
    virtual ErrorType flushRxBuffer() = 0;

    private:
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override {
        return txBlocking(data, timeout);

    }
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override {
        return rxBlocking(buffer, timeout);
    }
    virtual ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback = nullptr) override {
        return txNonBlocking(data, callback);
    }
    virtual ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback = nullptr) override {
        return rxNonBlocking(buffer, callback);
    }
};

#endif //__IC_COMMUNICATION_PROTOCOL_HPP__
