/***************************************************************************//**
* @author   Ben Haubrich
* @file     CommunicationProtocol.hpp
* @details  Interface for communication that involves sending and receiving bytes of data.
* @ingroup  Abstractions
*******************************************************************************/
#ifndef __COMMUNICATION_PROTOCOL_HPP__
#define __COMMUNICATION_PROTOCOL_HPP__

//AbstractionLayer
#include "EventQueue.hpp"
//C++
#include <memory>

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
    virtual ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) = 0;
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
    virtual ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) = 0;
};

#endif //__COMMUNICATION_PROTOCOL_HPP__
