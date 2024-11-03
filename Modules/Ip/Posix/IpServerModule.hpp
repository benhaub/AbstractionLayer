#ifndef __IP_SERVER_MODULE_HPP__
#define __IP_SERVER_MODULE_HPP__

//AbstractionLayer
#include "IpServerAbstraction.hpp"
//Posix
#include <sys/socket.h>

class IpServer : public IpServerAbstraction {

    public:
    ErrorType listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) override;
    ErrorType acceptConnection(const Socket socket) override;
    ErrorType closeConnection() override;
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override;
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override;
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback = nullptr) override;
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback = nullptr) override;

    private:
    int toPosixFamily(IpServerSettings::Version version) {
        switch (version) {
            case IpServerSettings::Version::IPv4:
                return AF_INET;
            case IpServerSettings::Version::IPv6:
                return AF_INET6;
            default:
                return AF_UNSPEC;
        }
    }

    int toPosixSocktype(IpServerSettings::Protocol protocol) {
        switch (protocol) {
            case IpServerSettings::Protocol::Tcp:
                return SOCK_STREAM;
            case IpServerSettings::Protocol::Udp:
                return SOCK_DGRAM;
            default:
                return SOCK_RAW;
        }
    }
};

#endif // __IP_SERVER_MODULE_HPP__