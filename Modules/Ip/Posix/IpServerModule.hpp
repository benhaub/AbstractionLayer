#ifndef __IP_SERVER_MODULE_HPP__
#define __IP_SERVER_MODULE_HPP__

//AbstractionLayer
#include "IpServerAbstraction.hpp"

class IpServer final : public IpServerAbstraction {

    public:
    IpServer() : IpServerAbstraction() {}

    ErrorType listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port) override;
    ErrorType acceptConnection(Socket &socket, const Milliseconds timeout) override;
    ErrorType closeConnection(const Socket socket) override;
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout, const Socket socket) override;
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout, Socket &socket) override;

    inline bool connectionsAcceptedIsAtMaximum() const {
        constexpr Count MaxConnections = 10;
        return _connectedSockets.size() >= MaxConnections;
    }
};

#endif // __IP_SERVER_MODULE_HPP__