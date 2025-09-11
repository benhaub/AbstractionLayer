#ifndef __WIFI_MODULE_HPP__
#define __WIFI_MODULE_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "WifiAbstraction.hpp"
//C++
#include <memory>
#include <string>

class Wifi final : public WifiAbstraction {

    public:
    Wifi() : WifiAbstraction() {}

    ErrorType init() override;
    ErrorType networkUp() override;
    ErrorType networkDown() override;
    ErrorType connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &sock, const Milliseconds timeout) override;
    ErrorType disconnect(const Socket &socket) override;
    ErrorType listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port, Socket &listenerSocket) override;
    ErrorType acceptConnection(const Socket &listenerSocket, Socket &newSocket, const Milliseconds timeout) override;
    ErrorType closeConnection(const Socket socket) override;
    ErrorType transmit(std::string_view frame, const Socket socket, const Milliseconds timeout) override;
    ErrorType receive(char *frameBuffer, const size_t bufferSize, const Socket socket, Bytes &read, const Milliseconds timeout) override;
    ErrorType getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) override;
    ErrorType getSignalStrength(DecibelMilliWatts &signalStrength) override;

    ErrorType radioOn() override;
    ErrorType radioOff() override;
    ErrorType setSsid(WifiTypes::Mode mode, const StaticString::Data<WifiTypes::MaxSsidLength> &ssid) override;
    ErrorType setPassword(WifiTypes::Mode mode, const StaticString::Data<WifiTypes::MaxPasswordLength> &password) override;
    ErrorType setMode(WifiTypes::Mode mode) override;
    ErrorType setAuthMode(WifiTypes::AuthMode authMode) override;
};

#endif // __WIFI_MODULE_HPP__
