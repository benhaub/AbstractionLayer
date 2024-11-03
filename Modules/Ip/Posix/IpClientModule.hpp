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
//Posix
#include <sys/socket.h>

class IpClient : public IpClientAbstraction {

    public:
    IpClient() : IpClientAbstraction() {};
    ~IpClient() = default;

    ErrorType connectTo(const std::string &hostname, const Port port, const IpClientSettings::Protocol protocol, const IpClientSettings::Version version, Socket &sock, const Milliseconds timeout) override;
    ErrorType disconnect() override;
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback = nullptr) override;
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback = nullptr) override;

    private:
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override;
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override;

    int toPosixFamily(IpClientSettings::Version version) {
        switch (version) {
            case IpClientSettings::Version::IPv4:
                return AF_INET;
            case IpClientSettings::Version::IPv6:
                return AF_INET6;
            default:
                return AF_UNSPEC;
        }
    }

    int toPosixSocktype(IpClientSettings::Protocol protocol) {
        switch (protocol) {
            case IpClientSettings::Protocol::Tcp:
                return SOCK_STREAM;
            case IpClientSettings::Protocol::Udp:
                return SOCK_DGRAM;
            default:
                return SOCK_RAW;
        }
    }
};

#endif // __IP_CLIENT_MODULE_HPP__