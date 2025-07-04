/***************************************************************************//**
* @author   Ben Haubrich
* @file     IpCellularClientModule.hpp
* @details  Cellular IP client for Quectel EC21A.
* @ingroup  QuectelEC21A Modules
*******************************************************************************/
#ifndef __IP_CELLULAR_CLIENT_MODULE_HPP__
#define __IP_CELLULAR_CLIENT_MODULE_HPP__

//AbstractionLayer
#include "IpClientAbstraction.hpp"

class IpCellularClient final : public IpClientAbstraction {

    public:
    IpCellularClient() : IpClientAbstraction() {}

    ErrorType connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &socket, const Milliseconds timeout) override;
    ErrorType disconnect() override;
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override;
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override;
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;
};

#endif // __IP_CELLULAR_CLIENT_MODULE_HPP__