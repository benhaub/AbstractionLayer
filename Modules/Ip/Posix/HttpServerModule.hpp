#ifndef __HTTP_SERVER_MODULE_HPP__
#define __HTTP_SERVER_MODULE_HPP__

//AbstractionLayer
#include "HttpServerAbstraction.hpp"
#include "IpServerModule.hpp"

class HttpServer : public HttpServerAbstraction {

    public:
    HttpServer() : HttpServerAbstraction() {
        _ipServer = std::make_unique<IpServer>();
        assert(nullptr != _ipServer.get());
    }
    virtual ~HttpServer() = default;

    ErrorType listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) override;
    ErrorType acceptConnection(Socket &socket) override;
    ErrorType closeConnection() override;
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override { return ErrorType::NotSupported; }
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override { return ErrorType::NotSupported; }
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override { return ErrorType::NotSupported; }
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override { return ErrorType::NotSupported; }

    ErrorType sendBlocking(const HttpServerTypes::Response &response, const Milliseconds timeout) override;
    ErrorType receiveBlocking(HttpServerTypes::Request &request, const Milliseconds timeout) override;
    ErrorType sendNonBlocking(const std::shared_ptr<HttpServerTypes::Response> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveNonBlocking(std::shared_ptr<HttpServerTypes::Request> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;

    private:
    std::unique_ptr<IpServer> _ipServer;
};

#endif // __HTTP_SERVER_MODULE_HPP__