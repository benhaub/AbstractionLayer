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
     * @param[in] callback The callback to call when the receive is complete
     * @returns ErrorType::Success if the request could be received
     * @returns ErrorType::Timeout if the receive could not be completed within the given timeout
     * @returns ErrorType::LimitReached if the event queue is full
     */
    virtual ErrorType receiveNonBlocking(std::shared_ptr<HttpTypes::Request> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<HttpTypes::Request> buffer)> callback) = 0;

    /**
     * @brief Read the request headers from a request to an http server.
     * @pre The size of the messaage body will determine how much data is read at once. If any of the message body is read while storing setting
     *      the contents of HttpTypes::Request, the part read will be saved to the message body.
     * @param[out] request The request headers from the request
     * @param[in] timeout The time to wait to receive data.
     * @param[out] sock The socket from which the data was received.
     * @param[in] networkReceiveFunction The function to call to receive data. If nullptr, IpClient::receiveBlocking will be used.
     * @returns ErrorType::Success if the response headers were read
     * @returns Any errors returned by IpClient::receiveBlocking
     * @code
     *   ErrorType error;
     *   HttpTypes::Request request;
     *   constexpr Bytes maxBufferSize = 512;
     *   request.messageBody.resize(maxBufferSize);
     *   std::string messageBody(maxBufferSize, 0);
     *   messageBody.resize(0);
     *   Milliseconds timeout = 1;

     *   Socket socketReceivedFrom = -1;

     *   error = httpServer().receiveBlocking(request, timeout, socketReceivedFrom);
     *   messageBody.append(request.messageBody);

     *   while (messageBody.size() < request.headers.contentLength && ErrorType::Success == error) {
     *       request.messageBody.resize(maxBufferSize);

     *       error = httpServer().receiveBlocking(request, timeout, socketReceivedFrom);

     *       if (ErrorType::Success == error) {
     *           messageBody.append(request.messageBody);
     *       }
     *   }
     *   request.messageBody.swap(messageBody);
     * @endcode
     */
    ErrorType readRequestHeaders(HttpTypes::Request &request, const Milliseconds timeout, Socket &sock, std::function<ErrorType(std::string &buffer, const Milliseconds timeout)> networkReceiveFunction = nullptr) {
        std::string &buffer = request.messageBody;
        const Bytes bufferSize = buffer.size();
        assert(bufferSize > 1 && "Buffer size must be greater than 1 in order to detect the end of the request headers");

        auto receive = [&](std::string &buffer) -> ErrorType {
            ErrorType error = ErrorType::Failure;

            if (nullptr == networkReceiveFunction) {
                error = _ipServer->receiveBlocking(buffer, timeout, sock);
            }
            else {
                error = networkReceiveFunction(buffer, timeout);
            }

            return error;
        };

        ErrorType error = receive(buffer);

        if (ErrorType::Success == error) {
            constexpr char endOfRequestHeaders[] = "\r\n\r\n";

            size_t requestBodyBegin = buffer.find(endOfRequestHeaders);
            const bool theBufferWasNotLargeEnoughToReadTheHeaderInOneGo = std::string::npos == requestBodyBegin;
            if (theBufferWasNotLargeEnoughToReadTheHeaderInOneGo) {
                //Extra buffer to store the request header fragments.
                std::string requestBuffer = buffer;

                do {
                    buffer.resize(bufferSize);
                    error = receive(buffer);

                    if (ErrorType::Success == error) {
                        requestBuffer.append(buffer);
                        requestBodyBegin = requestBuffer.find(endOfRequestHeaders);
                    }

                    
                } while (ErrorType::Success == error && std::string::npos == requestBodyBegin);

                if (ErrorType::Success == error) {
                    buffer.swap(requestBuffer);
                }
            }

            if (std::string::npos != requestBodyBegin) {
                //If any of the body was read while extracting the request headers, remove everything except the message body.
                error = toHttpRequest(buffer, request);
                buffer.erase(0, requestBodyBegin + sizeof(endOfRequestHeaders)-1);
            }
        }

        return error;
    }

    protected:
    /// @brief The Ip server to perform lower level network communications on.
    IpServerAbstraction *_ipServer;
};

#endif // __HTTP_SERVER_SBSTRACTION_HPP__