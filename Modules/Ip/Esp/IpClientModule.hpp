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
#include "lwip/sockets.h"

class IpClient final : public IpClientAbstraction {

    public:
    IpClient() : IpClientAbstraction() {};

    ErrorType connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout) override;
    ErrorType disconnect() override;
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout);
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout);

    private:
    int toPosixFamily(IpTypes::Version version) {
        switch (version) {
            case IpTypes::Version::IPv4:
                return AF_INET;
            case IpTypes::Version::IPv6:
                return AF_INET6;
            default:
                return AF_UNSPEC;
        }
    }

    int toPosixSocktype(IpTypes::Protocol protocol) {
        switch (protocol) {
            case IpTypes::Protocol::Tcp:
                return SOCK_STREAM;
            case IpTypes::Protocol::Udp:
                return SOCK_DGRAM;
            default:
                return SOCK_RAW;
        }
    }
};

#endif // __IP_CLIENT_MODULE_HPP__