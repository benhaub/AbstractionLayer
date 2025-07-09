#ifndef __HTTP_SERVER_MODULE_HPP__
#define __HTTP_SERVER_MODULE_HPP__

//AbstractionLayer
#include "HttpServerAbstraction.hpp"

class HttpServer final : public HttpServerAbstraction {

    public:
    HttpServer(IpServerAbstraction &ipServer) : HttpServerAbstraction(ipServer) {}

    ErrorType listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) override;
    ErrorType acceptConnection(Socket &socket, const Milliseconds timeout) override;
    ErrorType closeConnection(const Socket socket) override;

    ErrorType sendBlocking(const HttpTypes::Response &response, const Milliseconds timeout, const Socket socket) override;
    ErrorType receiveBlocking(HttpTypes::Request &request, const Milliseconds timeout, Socket &socket) override;
    ErrorType sendNonBlocking(const std::shared_ptr<HttpTypes::Response> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveNonBlocking(std::shared_ptr<HttpTypes::Request> buffer, const Milliseconds timeout, Socket &socket, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<HttpTypes::Request> buffer)> callback) override;
};

#endif // __HTTP_SERVER_MODULE_HPP__