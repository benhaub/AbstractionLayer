#ifndef __HTTP_CLIENT_MODULE_HPP__
#define __HTTP_CLIENT_MODULE_HPP__

//AbstractionLayer
#include "HttpClientAbstraction.hpp"

class HttpClient final : public HttpClientAbstraction {

    public:
    HttpClient(IpClientAbstraction &ipClient) : HttpClientAbstraction(ipClient) {}

    ErrorType connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout) override;
    ErrorType disconnect() override;
    ErrorType sendBlocking(const HttpTypes::Request &request, const Milliseconds timeout) override;
    ErrorType receiveBlocking(HttpTypes::Response &response, const Milliseconds timeout) override;
    ErrorType sendNonBlocking(const std::shared_ptr<HttpTypes::Request> request, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveNonBlocking(std::shared_ptr<HttpTypes::Response> response, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<HttpTypes::Request> buffer)> callback) override;
};

#endif // __HTTP_CLIENT_MODULE_HPP__
