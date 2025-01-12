#ifndef __IP_SERVER_MODULE_HPP__
#define __IP_SERVER_MODULE_HPP__

//AbstractionLayer
#include "IpServerAbstraction.hpp"
//Posix
#include <sys/socket.h>

class IpServer : public IpServerAbstraction {

    public:
    ErrorType listenTo(const IpServerSettings::Protocol protocol, const IpServerSettings::Version version, const Port port) override;
    ErrorType acceptConnection(Socket &socket, const Milliseconds timeout) override;
    ErrorType closeConnection(const Socket socket) override;
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket) override;
    ErrorType sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const Socket socket, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket) override;
    ErrorType receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, const Socket socket, std::shared_ptr<std::string> buffer)> callback) override;

    private:
    /// @brief Index of the last socket we received from so that we can give all the connected sockets a chance at getting
    //their data received.
    size_t _previousReceivedSocketIndex = 0;

    inline int toPosixFamily(IpServerSettings::Version version) const {
        switch (version) {
            case IpServerSettings::Version::IPv4:
                return AF_INET;
            case IpServerSettings::Version::IPv6:
                return AF_INET6;
            default:
                return AF_UNSPEC;
        }
    }

    inline int toPosixSocktype(IpServerSettings::Protocol protocol) const {
        switch (protocol) {
            case IpServerSettings::Protocol::Tcp:
                return SOCK_STREAM;
            case IpServerSettings::Protocol::Udp:
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