#ifndef __IP_SERVER_MODULE_HPP__
#define __IP_SERVER_MODULE_HPP__

//AbstractionLayer
#include "IpServerAbstraction.hpp"
//ESP
#include "lwip/sockets.h"

class IpServer final : public IpServerAbstraction {

    public:
    IpServer() : IpServerAbstraction() {};

    ErrorType listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) override;
    ErrorType acceptConnection(Socket &socket, const Milliseconds timeout) override;
    ErrorType closeConnection(const Socket socket) override;
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket) override;
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket) override;

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

    inline bool connectionsAcceptedIsAtMaximum() const {
        constexpr Count MaxConnections = 10;
        return _connectedSockets.size() >= MaxConnections;
    }
};

#endif // __IP_SERVER_MODULE_HPP__