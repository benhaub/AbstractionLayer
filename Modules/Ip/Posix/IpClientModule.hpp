/***************************************************************************//**
* @author   Ben Haubrich
* @file     IpClientModule.hpp
* @details  IP client for posix compliant systems.
* @ingroup  PosixModules
*******************************************************************************/
#ifndef __IP_CLIENT_MODULE_HPP__
#define __IP_CLIENT_MODULE_HPP__

//AbstractionLayer
#include "IpClientAbstraction.hpp"

class IpClient final : public IpClientAbstraction {

    public:
    IpClient() : IpClientAbstraction() {};

    ErrorType connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout) override;
    ErrorType disconnect() override;
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override;
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override;
};

#endif // __IP_CLIENT_MODULE_HPP__