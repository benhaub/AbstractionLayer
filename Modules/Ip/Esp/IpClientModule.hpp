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

class IpClient : public IpClientAbstraction {

    public:
    IpClient() : IpClientAbstraction() {};
    ~IpClient() = default;

    ErrorType connectTo(const std::string &hostname, const Port port, const IpClientTypes::Protocol protocol, const IpClientTypes::Version version, Socket &sock, const Milliseconds timeout) override;
    ErrorType disconnect() override;
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout);
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout);

    private:
    int toPosixFamily(IpClientTypes::Version version) {
        switch (version) {
            case IpClientTypes::Version::IPv4:
                return AF_INET;
            case IpClientTypes::Version::IPv6:
                return AF_INET6;
            default:
                return AF_UNSPEC;
        }
    }

    int toPosixSocktype(IpClientTypes::Protocol protocol) {
        switch (protocol) {
            case IpClientTypes::Protocol::Tcp:
                return SOCK_STREAM;
            case IpClientTypes::Protocol::Udp:
                return SOCK_DGRAM;
            default:
                return SOCK_RAW;
        }
    }
};

#endif // __IP_CLIENT_MODULE_HPP__