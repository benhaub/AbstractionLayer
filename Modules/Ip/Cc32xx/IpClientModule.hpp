#ifndef __IP_CLIENT_MODULE_HPP__
#define __IP_CLIENT_MODULE_HPP__

//AbstractionLayer
#include "IpClientAbstraction.hpp"
//TI
#include "ti/drivers/net/wifi/slnetifwifi.h"

class IpClient final : public IpClientAbstraction {

    public:
    IpClient() : IpClientAbstraction() {};

    ErrorType connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &sock, const Milliseconds timeout) override;
    ErrorType disconnect() override;
    ErrorType sendBlocking(const std::string &data, const Milliseconds timeout) override;
    ErrorType receiveBlocking(std::string &buffer, const Milliseconds timeout) override;

    private:
    constexpr _i16 toSimplelinkDomain(const IpTypes::Version &version, ErrorType &error) {
        error = ErrorType::Success;

        switch (version) {
            case IpTypes::Version::IPv4:
                return SLNETSOCK_AF_INET;
            case IpTypes::Version::IPv6:
                return SLNETSOCK_AF_INET6;
            default:
                error = ErrorType::NotSupported;
                return SLNETSOCK_AF_INET;
        }
    }

    constexpr _i16 toSimpleLinkProtocol(const IpTypes::Protocol &protocol, ErrorType &error) {
        error = ErrorType::Success;

        switch (protocol) {
            case IpTypes::Protocol::Tcp:
                return SL_IPPROTO_TCP;
            case IpTypes::Protocol::Udp:
                return SL_IPPROTO_UDP;
            default:
                error = ErrorType::NotSupported;
                return SL_IPPROTO_UDP;
        }
    }

    constexpr _i16 toSimpleLinkType(const IpTypes::Protocol &protocol, ErrorType &error) {
        error = ErrorType::Success;

        switch (protocol) {
            case IpTypes::Protocol::Tcp:
                return SLNETSOCK_SOCK_STREAM;
            case IpTypes::Protocol::Udp:
                return SLNETSOCK_SOCK_DGRAM;
            default:
                error = ErrorType::NotSupported;
                return SLNETSOCK_SOCK_RAW;
        }
    }

    constexpr _i16 toSimpleLinkProtocolFamily(const IpTypes::Version &version, ErrorType &error) {
        return toSimplelinkDomain(version, error);
    }
};

#endif // __IP_CLIENT_MODULE_HPP__
