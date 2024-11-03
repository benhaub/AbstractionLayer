#ifndef __IP_CELLULAR_SERVER_MODULE_HPP__
#define __IP_CELLULAR_SERVER_MODULE_HPP__

#include "IpServerAbstraction.hpp"

class IpCellularServer : public IpServerAbstraction {

    public:
    ErrorType listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) override;
    ErrorType acceptConnection(const Socket socket) override;
    ErrorType closeConnection() override;
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override;
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override;
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback = nullptr) override;
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback = nullptr) override;
};

#endif // __IP_CELLULAR_SERVER_MODULE_HPP__