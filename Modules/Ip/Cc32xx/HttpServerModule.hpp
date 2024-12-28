#ifndef __HTTP_SERVER_MODULE_HPP__
#define __HTTP_SERVER_MODULE_HPP__

//AbstractionLayer
#include "HttpServerAbstraction.hpp"
#include "OperatingSystemModule.hpp"
//TI Drivers
#include "ti/drivers/net/wifi/simplelink.h"

//Global because the SimpleLink C handlers are outside the scope of this class and need to be able to access the queue.
static constexpr char SimpleLinkEventQueue[] = "SimpleLinkEventQueue";

class HttpServer : public HttpServerAbstraction {
    public:
    HttpServer() : HttpServerAbstraction() {
        OperatingSystem::Instance().createQueue(SimpleLinkEventQueue, sizeof(SlNetAppRequest_t), 10);
    }
    virtual ~HttpServer() = default;

    ErrorType listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) override;
    ErrorType acceptConnection(Socket &socket) override;
    ErrorType closeConnection() override;
    //These functions could perhaps be used to implement websockets
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override { return ErrorType::NotSupported; }
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override { return ErrorType::NotSupported; }
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override { return ErrorType::NotSupported; }
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override { return ErrorType::NotSupported; }

    ErrorType sendBlocking(HttpServerTypes::Response response, Milliseconds timeout) override;
    ErrorType receiveBlocking(HttpServerTypes::Request &request, const Milliseconds timeout) override;
    ErrorType sendNonBlocking(const std::shared_ptr<HttpServerTypes::Response> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveNonBlocking(std::shared_ptr<HttpServerTypes::Request> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;
};

#endif // __HTTP_SERVER_MODULE_HPP__