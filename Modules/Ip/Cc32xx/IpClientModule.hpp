#ifndef __IP_CLIENT_MODULE_HPP__
#define __IP_CLIENT_MODULE_HPP__

//Foundation
#include "IpClientAbstraction.hpp"

class IpClient : public IpClientAbstraction {

    public:
    IpClient() : IpClientAbstraction() {};
    ~IpClient() = default;

    ErrorType connectTo(const std::string &hostname, const Port port, const IpClientTypes::Protocol protocol, const IpClientTypes::Version version, Socket &sock, const Milliseconds timeout) override;
    ErrorType disconnect() override;
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;

    private:
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override;
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override;
};

#endif // __IP_CLIENT_MODULE_HPP__