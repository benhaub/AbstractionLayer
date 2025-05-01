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
#include "Error.hpp"
//C++
#include <memory>
#include <string>

class Wifi final : public WifiAbstraction {

    public:
    Wifi() : WifiAbstraction() {
        //Wifi on a posix system is always up.
        _status.isUp = true;
    }

    ErrorType init() override;
    ErrorType networkUp() override;
    ErrorType networkDown() override;
    ErrorType txBlocking(const std::string &frame, const Socket socket, const Milliseconds timeout) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> frameBuffer, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> frameBuffer)> callback) override;
    ErrorType getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) override;
    ErrorType getSignalStrength(DecibelMilliWatts &signalStrength) override;

    ErrorType mainLoop() override;

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
};

#endif // __WIFI_MODULE_HPP__
