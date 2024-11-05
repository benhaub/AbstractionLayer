/***************************************************************************//**
* @author   Ben Haubrich
* @file     CommunicationProtocol.hpp
* @details  \b Synopsis: \n Interface for communication that involves sending and receiving bytes of data.
* @ingroup  AbstractionLayer
*******************************************************************************/
#ifndef __COMMUNICATION_PROTOCOL_HPP__
#define __COMMUNICATION_PROTOCOL_HPP__

//C++
#include <memory>
#include <functional>
//Foundation
#include "Types.hpp"
#include "Error.hpp"
#include "EventQueue.hpp"

/**
 * @class CommunicationProtocol
 * @brief Interface for communication that involves sending and receiving bytes of data.
*/
class CommunicationProtocol : public EventQueue {

    public:
    /**
     * @brief Constructor.
    */
    CommunicationProtocol() : EventQueue() {}
    /**
     * @brief Destructor.
    */
    virtual ~CommunicationProtocol() = default;

    /**
     * @brief Sends data.
     * @param[in] data The data to send.
     * @param[in] timeout The timeout in milliseconds.
     * @returns ErrorType::Success if the data was sent.
     * @returns ErrorType::Failure if the data was not sent.
     * @returns ErrorType::Timeout if the timeout was reached.
     * @post The amount of data to transmit is equal to the size of the data. See std::string::size(), std::string::resize().
    */
    virtual ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) = 0;
    /**
     * @brief Sends data.
     * @param[in] data The data to send.
     * @param[in] callback The callback to call when the data is sent.
     * @returns ErrorType::Success if the data was sent.
     * @returns ErrorType::Failure if the data was not sent.
     * @post The callback will be called when the data has been sent. The bytes written is valid if and only if error is equal to ErrorType::Success.
    */
    virtual ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) = 0;
    /**
     * @brief Receives data.
     * @param[in] buffer The data to receive.
     * @param[in] timeout The timeout in milliseconds.
     * @returns ErrorType::Success if the data was received.
     * @returns ErrorType::Failure if the data was not received.
     * @returns ErrorType::Timeout if the timeout was reached.
     * @post The amount of data received is equal to the size of the data. See std::string::size(), std::string::resize().
    */
    virtual ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) = 0;
    /**
     * @brief Receives data.
     * @param[in] buffer The buffer to receive the data into.
     * @param[in] callback The callback to call when the data has been received.
     * @post The callback will be called when the data has been received. The buffer is valid if and only if error is equal to ErrorType::Success.
    */
    virtual ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) = 0;
};

#endif //__COMMUNICATION_PROTOCOL_HPP__