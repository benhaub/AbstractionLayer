/***************************************************************************//**
* @author  Ben Haubrich
* @file    HttpServerAbstraction.hpp
* @details Interface for creating an http server on any network
* @ingroup Abstractions
*******************************************************************************/
#ifndef __HTTP_SERVER_SBSTRACTION_HPP__
#define __HTTP_SERVER_SBSTRACTION_HPP__

//AbstractionLayer
#include "HttpTypes.hpp"
#include "NetworkAbstraction.hpp"
//C++
#include <memory>

/**
 * @class HttpServerAbstraction
 * @brief An Http server
 */
class HttpServerAbstraction {

    public:
    /// @brief Destructor
    virtual ~HttpServerAbstraction() = default;

    /// @brief Tag for logging
    static constexpr char TAG[] = "HttpServer";

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
     * @brief Set the network.
     * @param The network that this server will use to communicate on.
     */
    virtual void setNetwork(NetworkAbstraction &network) {
        _network = &network;
    }

    /**
     * @brief Read the request headers from a request to an http server.
     * @pre The size of the messaage body will determine how much data is read at once. If any of the message body is read while storing setting
     *      the contents of HttpTypes::Request, the part read will be saved to the message body.
     * @param[out] request The request headers from the request
     * @param[in] timeout The time to wait to receive data.
     * @param[out] sock The socket from which the data was received.
     * @returns ErrorType::Success if the response headers were read
     * @returns ErrorType::LimitReached if the buffer was not long enough to fully read the longest header present in the request.
                If your the client, make the request again with a larger buffer. If you're the server, you can send a 431 or 414.
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
    ErrorType readRequestHeaders(HttpTypes::Request &request, const Milliseconds timeout) {
        std::string &buffer = request.messageBody;
        assert(buffer.size() >= 4 && "Buffer size must be at least 4 bytes in order to detect the end of the request headers");
        ErrorType error = ErrorType::Negative;

        constexpr char requestHeaderSeperator[] = "\r\n";
        constexpr char endOfRequestHeaders[] = "\r\n\r\n";
        std::string lastPartialHeader;

        buffer.insert(0, lastPartialHeader);
        error = toHttpRequest(buffer, request);

        if (ErrorType::Success == error) {

            const bool requestHeaderPartiallyRead = 0 != buffer.compare(buffer.size() - sizeof(requestHeaderSeperator)-1,
                                                                sizeof(requestHeaderSeperator)-1,
                                                                requestHeaderSeperator,
                                                                sizeof(requestHeaderSeperator)-1);
            if (requestHeaderPartiallyRead) {
                const size_t beginningOfLastCompleteHeader = buffer.rfind(requestHeaderSeperator) + sizeof(requestHeaderSeperator)-1;

                if (std::string::npos != beginningOfLastCompleteHeader) {
                    std::string_view partiallyReadHeader = std::string_view(buffer).substr(beginningOfLastCompleteHeader);
                    lastPartialHeader.assign(partiallyReadHeader);
                }
                else {
                    //We could try to store up the header fragments in a new buffer, but at what point do we stop?
                    //What if the header fragment reads into the next header? It's best to just give up here because it
                    //puts more control into the application. The software will be easier to debug and maintain when the
                    //application is able to make more decisions and has the most effect on the overall behaviour.
                    return ErrorType::LimitReached;
                }
            }
            else {
                lastPartialHeader.clear();
            }
        }

        const size_t requestBodyBegin = buffer.find(endOfRequestHeaders);
        const bool entireRequestRead = std::string::npos != requestBodyBegin;

        //If any of the body was read while extracting the request headers, remove everything except the message body.
        if (requestBodyBegin < buffer.size() && entireRequestRead) {
            buffer.erase(0, requestBodyBegin + sizeof(endOfRequestHeaders)-1);
            error = ErrorType::Negative;
        }

        return error;
    }

    protected:
    NetworkAbstraction *_network = nullptr;
};

#endif // __HTTP_SERVER_SBSTRACTION_HPP__