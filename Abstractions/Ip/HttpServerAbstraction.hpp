/***************************************************************************//**
* @author  Ben Haubrich
* @file    HttpServerAbstraction.hpp
* @details Interface for creating an http server on any network
* @ingroup Abstractions
*******************************************************************************/
#ifndef __HTTP_SERVER_SBSTRACTION_HPP__
#define __HTTP_SERVER_SBSTRACTION_HPP__

//AbstractionLayer
#include "IpServerAbstraction.hpp"
#include "HttpTypes.hpp"

/**
 * @class HttpServerAbstraction
 * @brief An Http server
 */
class HttpServerAbstraction {

    public:

    /// @brief Constructor
    HttpServerAbstraction(IpServerAbstraction &ipServer) : _ipServer(&ipServer) {}
    /// @brief Destructor
    virtual ~HttpServerAbstraction() = default;

    /// @brief Tag for logging
    static constexpr char TAG[] = "HttpServer";

    void printStatus() {
        _ipServer->printStatus();
    }

    /**
     * @brief Listen for connections on a port
     * @param[in] protocol The protocol to use for the connection
     * @sa IpTypes::Protocol
     * @param[in] version The version to use for the connection
     * @param[in] port The port to listen to
     * @sa IpTypes::Version
    */
    virtual ErrorType listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) = 0;
    /**
     * @brief Accept a connection from a client connecting to the socket given
     * @param[out] socket The socket that the connection was accepted on
     * @param[in] timeout The time to wait to accept a connection.
     * @returns ErrorType::Success on success
     * @returns ErrorType::LimitReached if the maximum number of connections has been accepted
     * @returns ErrorType::Timeout if no connections were accepted within the given timeout.
     * @returns ErrorType::Failure otherwise
    */
    virtual ErrorType acceptConnection(Socket &socket, const Milliseconds timeout) = 0;
    /**
     * @brief Close the connection
     * @param[in] socket The socket to close
     * @returns ErrorType::Success on success
    */
    virtual ErrorType closeConnection(const Socket socket) = 0;
    /**
     * @brief Send a response
     * @param[in] response The response to send
     * @param[in] timeout The timeout
     * @param[in] socket The socket to use
     * @returns ErrorType::Success if the response could be sent
     * @returns ErrorType::Timeout if the send could not be completed within the given timeout
     */
    virtual ErrorType sendBlocking(const HttpTypes::Response &response, const Milliseconds timeout, const Socket socket) = 0;
    /**
     * @brief Receive a request
     * @param[in] request The request to receive
     * @param[in] timeout The timeout
     * @param[out] socket If negative, will contain the socket in which the data was received from. Otherwise it will be the socket in which data is explicitely received from.
     * @returns ErrorType::Success if the request could be received
     * @returns ErrorType::Timeout if the receive could not be completed within the given timeout
     */
    virtual ErrorType receiveBlocking(HttpTypes::Request &request, const Milliseconds timeout, Socket &socket) = 0;
    /**
     * @brief Send a response
     * @param[in] data The data to send
     * @param[in] timeout The timeout
     * @param[in] socket The socket to use
     * @param[in] callback The callback to call when the send is complete
     * @returns ErrorType::Success if the response could be sent
     * @returns ErrorType::Timeout if the send could not be completed within the given timeout
     * @returns ErrorType::LimitReached if the event queue is full
     */
    virtual ErrorType sendNonBlocking(const std::shared_ptr<HttpTypes::Response> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) = 0;
    /**
     * @brief Receive a request
     * @param[in] buffer The buffer to receive the request into
     * @param[in] timeout The timeout
     * @param[out] socket If negative, will contain the socket in which the data was received from. Otherwise it will be the socket in which data is explicitely received from.
     * @param[in] callback The callback to call when the receive is complete
     * @returns ErrorType::Success if the request could be received
     * @returns ErrorType::Timeout if the receive could not be completed within the given timeout
     * @returns ErrorType::LimitReached if the event queue is full
     */
    virtual ErrorType receiveNonBlocking(std::shared_ptr<HttpTypes::Request> buffer, const Milliseconds timeout, Socket &socket, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<HttpTypes::Request> buffer)> callback) = 0;

    protected:
    /// @brief The Ip server to perform lower level network communications on.
    IpServerAbstraction *_ipServer;
};

#endif // __HTTP_SERVER_SBSTRACTION_HPP__