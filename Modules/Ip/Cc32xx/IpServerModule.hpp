#ifndef __IP_SERVER_MODULE_HPP__
#define __IP_SERVER_MODULE_HPP__

//AbstractionLayer
#include "IpServerAbstraction.hpp"
#include "OperatingSystemModule.hpp"
//TI Drivers
#include "ti/drivers/net/wifi/simplelink.h"

//Global because the SimpleLink C handlers are outside the scope of this class and need to be able to access the queue.
static constexpr char SimpleLinkEventQueue[] = "SimpleLinkEventQueue";

class IpServer : public IpServerAbstraction {

    public:
    IpServer() : IpServerAbstraction() {
        OperatingSystem::Instance().createQueue(SimpleLinkEventQueue, sizeof(SlNetAppRequest_t), 10);
    }
    ~IpServer() = default;

    ErrorType listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) override;
    ErrorType acceptConnection(Socket &socket) override;
    ErrorType closeConnection() override;
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override;
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override;
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;
};

#endif // __IP_SERVER_MODULE_HPP__