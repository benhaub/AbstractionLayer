/***************************************************************************//**
* @author  Ben Haubrich
* @file    HttpClientAbstraction.hpp
* @details Interface for creating an http client on any network
* @ingroup Abstractions
*******************************************************************************/
#ifndef __HTTP_CLIENT_ABSTRACTION_HPP__
#define __HTTP_CLIENT_ABSTRACTION_HPP__

//AbstractionLayer
#include "IpClientAbstraction.hpp"
#include "HttpTypes.hpp"

/**
 * @class HttpClientAbstraction
 * @brief An Http client
 */
class HttpClientAbstraction {

    public:
    /// @brief Constructor
    HttpClientAbstraction(IpClientAbstraction &ipClient) : _ipClient(&ipClient) {}
    /// @brief Destructor
    virtual ~HttpClientAbstraction() = default;

    /// @brief Tag for logging
    static constexpr char TAG[] = "HttpClient";

    void printStatus() {
        _ipClient->printStatus();
    }

    /**
     * @brief Connect to a host
     * @param[in] hostname The hostname to connect to
     * @param[in] port The port to connect to
     * @param[in] protocol The protocol to use
     * @sa IpSettings::Protocol
     * @param[in] version The version to use
     * @sa IpTypes::Version
     * @param[out] socket The socket that was created
     * @param[in] timeout The amount of time to wait to connect to the host
     * @post The caller is blocked until the connection is made or the timeout is reached. The connection can still be made after the timeout is reached.
     * @returns Fnd::ErrorType::Success on success
     * @returns Fnd::ErrorType::Failure on failure
     * @returns Fnd::ErrorType::NotImplemented if not implemented
     * @returns Fnd::ErrorType::NotSupported if the network interface doesn't support the operation
    */
    virtual ErrorType connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout) = 0;
    /**
     * @brief Disconnect this client
     * @returns Fnd::ErrorType::Success on success
     * @returns Fnd::ErrorType::Failure on failure
     * @returns Fnd::ErrorType::NotImplemented if not implemented
    */
    virtual ErrorType disconnect() = 0;
    /**
     * @brief Send a request
     * @param[in] request The response to send
     * @param[in] timeout The timeout
     * @returns ErrorType::Success if the request could be sent
     * @returns ErrorType::Timeout if the send could not be completed within the given timeout
     */
    virtual ErrorType sendBlocking(const HttpTypes::Request &request, const Milliseconds timeout) = 0;
    /**
     * @brief Receive a response
     * @pre The message body of the response must have a non-zero size in order to receive data.
     * @param[in] request The response to receive
     * @param[in] timeout The timeout
     * @returns ErrorType::Success if the request could be received
     * @returns ErrorType::Timeout if the receive could not be completed within the given timeout
     * @attention Do not edit the contents of the response headers. For multiple calls to receiveBlocking,
     *            the contents are used to determine if the response headers have been read or not. If you
     *            edit the contents, then you will incur the penalty of attempting to parse response headers
     *            each time a segment of the message body is received.
     */
    virtual ErrorType receiveBlocking(HttpTypes::Response &response, const Milliseconds timeout) = 0;
    /**
     * @brief Send a request
     * @param[in] data The data to send
     * @param[in] timeout The timeout
     * @param[in] callback The callback to call when the send is complete
     * @returns ErrorType::Success if the response could be sent
     * @returns ErrorType::Timeout if the send could not be completed within the given timeout
     * @returns ErrorType::LimitReached if the event queue is full
     */
    virtual ErrorType sendNonBlocking(const std::shared_ptr<HttpTypes::Request> request, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) = 0;
    /**
     * @brief Receive a request
     * @param[in] response The buffer to receive the response into
     * @param[in] timeout The timeout
     * @param[in] callback The callback to call when the receive is complete
     * @returns ErrorType::Success if the request could be received
     * @returns ErrorType::Timeout if the receive could not be completed within the given timeout
     * @returns ErrorType::LimitReached if the event queue is full
     */
    virtual ErrorType receiveNonBlocking(std::shared_ptr<HttpTypes::Response> response, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<HttpTypes::Request> buffer)> callback) = 0;

    /**
     * @brief Read the response headers from a reply to an http request.
     * @param[out] reponse The response headers from the reply
     * @param[in] timeout The time to wait to receive data.
     * @param[in] networkReceiveFunction The function to call to receive data. If nullptr, IpClient::receiveBlocking will be used.
     * @returns ErrorType::Success if the response headers were read
     * @returns Any errors returned by IpClient::receiveBlocking
     */
    ErrorType readResponseHeaders(HttpTypes::Response &response, const Milliseconds timeout, std::function<ErrorType(std::string &buffer, const Milliseconds timeout)> networkReceiveFunction = nullptr) {
        std::string &buffer = response.messageBody;
        const Bytes bufferSize = buffer.size();
        ErrorType error = ErrorType::Failure;

        size_t responseBodyBegin = 0;
        do {
            buffer.resize(bufferSize);

            if (nullptr == networkReceiveFunction) {
                error = _ipClient->receiveBlocking(buffer, timeout);
            }
            else {
                error = networkReceiveFunction(buffer, timeout);
            }

            if (ErrorType::Success == error) {
                size_t contentLengthBegin = buffer.find("Content-Length:");
                size_t responseCodeBegin = buffer.find("HTTP/1.");
                if (std::string::npos != contentLengthBegin) {
                    contentLengthBegin += sizeof("Content-Length:");
                    size_t contentLengthEnd = buffer.find("\r\n", contentLengthBegin);
                    if (std::string::npos != contentLengthEnd) {
                        response.representationHeaders.contentLength = std::stoi(buffer.substr(contentLengthBegin, contentLengthEnd - contentLengthBegin));
                    }
                }
                if (std::string::npos != responseCodeBegin) {
                    //Plus one for whatever the 1.x version is.
                    responseCodeBegin += sizeof("HTTP/1.") + 1;
                    //Don't go to the \r\n because we don't want the string representation of the response code.
                    size_t responseCodeEnd = buffer.find(" ", responseCodeBegin);
                    if (std::string::npos != responseCodeEnd) {
                        response.statusLine.statusCode = static_cast<HttpTypes::StatusCode>(std::stoi(buffer.substr(responseCodeBegin, responseCodeEnd - responseCodeBegin)));
                    }
                }
            }

        } while (ErrorType::Success == error && std::string::npos == (responseBodyBegin = buffer.find("\r\n\r\n")));

        //If any of the body was read while extracting the response headers, remove everything except the message body.
        if (ErrorType::Success == error) {
            buffer.erase(0, responseBodyBegin + sizeof("\r\n\r\n")-1);
        }

        return error;
    }

    protected:
    /// @brief The Ip client to perform lower level network communications on.
    IpClientAbstraction *_ipClient;
};

#endif // __HTTP_CLIENT_ABSTRACTION_HPP__