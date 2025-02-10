#ifndef __IP_SERVER_MODULE_HPP__
#define __IP_SERVER_MODULE_HPP__

//AbstractionLayer
#include "IpServerAbstraction.hpp"
//Posix
#include <sys/socket.h>

class IpServer : public IpServerAbstraction {

    public:
    ErrorType listenTo(const IpServerTypes::Protocol protocol, const IpServerTypes::Version version, const Port port) override;
    ErrorType acceptConnection(Socket &socket, const Milliseconds timeout) override;
    ErrorType closeConnection(const Socket socket) override;
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket) override;
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket) override;
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, Socket &socket, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) override;

    private:
    inline int toPosixFamily(IpServerTypes::Version version) const {
        switch (version) {
            case IpServerTypes::Version::IPv4:
                return AF_INET;
            case IpServerTypes::Version::IPv6:
                return AF_INET6;
            default:
                return AF_UNSPEC;
        }
    }

    inline int toPosixSocktype(IpServerTypes::Protocol protocol) const {
        switch (protocol) {
            case IpServerTypes::Protocol::Tcp:
                return SOCK_STREAM;
            case IpServerTypes::Protocol::Udp:
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