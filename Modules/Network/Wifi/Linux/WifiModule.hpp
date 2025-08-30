/***************************************************************************//**
* @author   Ben Haubrich
* @file     WifiModule.hpp
* @details  Wifi for posix compliant systems.
* @ingroup  PosixModules
*******************************************************************************/
#ifndef __WIFI_MODULE_HPP__
#define __WIFI_MODULE_HPP__

//AbstractionLayer
#include "WifiAbstraction.hpp"
//Posix
#include <sys/socket.h>

class Wifi final : public WifiAbstraction {

    public:
    Wifi() : WifiAbstraction() {
        //Wifi on a posix system is always up.
        NetworkAbstraction::_status.isUp = true;
        _status.isProvisioned = true;
    }

    ErrorType init() override;
    ErrorType networkUp() override;
    ErrorType networkDown() override;
    ErrorType connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &sock, const Milliseconds timeout) override;
    ErrorType disconnect(const Socket &socket) override;
    ErrorType listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port, Socket &listenerSocket) override;
    ErrorType acceptConnection(const Socket &listenerSocket, Socket &newSocket, const Milliseconds timeout) override;
    ErrorType closeConnection(const Socket socket) override;
    ErrorType transmit(const std::string &frame, const Socket socket, const Milliseconds timeout) override;
    ErrorType receive(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) override;
    ErrorType getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) override;
    ErrorType getSignalStrength(DecibelMilliWatts &signalStrength) override;

    ErrorType radioOn() override { return ErrorType::NotAvailable; }
    ErrorType radioOff() override { return ErrorType::NotAvailable; }
    ErrorType setSsid(WifiTypes::Mode mode, const std::string &ssid) override { return ErrorType::NotAvailable; }
    ErrorType setPassword(WifiTypes::Mode mode, const std::string &password) override { return ErrorType::NotAvailable; }
    ErrorType setMode(WifiTypes::Mode mode) override { return ErrorType::NotAvailable; }
    ErrorType setAuthMode(WifiTypes::AuthMode authMode) override { return ErrorType::NotAvailable; }

    private:
    /**
     * @brief Given a hostname, get it's IP address
     * @param[in] host The hostname to get the IP address of
     * @param[out] The IP address of the host
     * @returns ErrorType::Success if the IP address was returned.
     * @returns ErrorType::Failure otherwise.
     */
    ErrorType hostToIp(const std::string_view host, std::array<char, NetworkTypes::Ipv4AddressStringSize> &ipAddress);
    /**
     * @brief Given a destination IP address, get the interface that the outgoing packet was routed to to reach the destination.
     * @param[in] ipAddress The destination IP address
     * @param[out] interface The interface that the packet was routed through to reach the destination
     * @returns ErrorType::Success if the interface was returned
     * @returns ErrorType::Failure if the interface was not returned.
     */
    ErrorType interfaceRoutedTo(const std::array<char, NetworkTypes::Ipv4AddressStringSize> &ipAddress, std::array<char, 16> &interface);

    /**
     * @brief Convert the IpTypes::Version to a POSIX family type.
     * @param[in] version The IpTypes::Version to convert
     * @returns The POSIX family type for the given IpTypes::Version
     */
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

    /**
     * @brief Convert the IpTypes::Protocol to a POSIX socket type.
     * @param[in] protocol The IpTypes::Protocol to convert
     * @returns The POSIX socket type for the given IpTypes::Protocol
     */
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

#endif // __WIFI_MODULE_HPP__
