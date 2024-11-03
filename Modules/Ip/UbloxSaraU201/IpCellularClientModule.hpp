#ifndef __IP_CELLULAR_CLIENT_MODULE_HPP__
#define __IP_CELLULAR_CLIENT_MODULE_HPP__

//Foundation
#include "IpClientAbstraction.hpp"

class IpCellularClient : public IpClientAbstraction {

    public:
    IpCellularClient() : IpClientAbstraction() {};
    ~IpCellularClient() = default;

    ErrorType connectTo(const std::string &hostname, const Port port, const IpClientSettings::Protocol protocol, const IpClientSettings::Version version, Socket &socket, const Milliseconds timeout) override;
    ErrorType disconnect() override;
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback = nullptr) override;
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback = nullptr) override;

    private:
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override;
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override;
};

#endif // __IP_CELLULAR_CLIENT_MODULE_HPP__