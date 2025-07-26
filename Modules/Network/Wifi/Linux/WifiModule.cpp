//AbstractionLayer
#include "WifiModule.hpp"
#include "Log.hpp"
//C++
#include <cassert>
#include <cstdio>
#include <limits>
#include <cstring>
//Posix
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <net/if.h>     //For getting the MAC address
#include <sys/ioctl.h>  //For getting the MAC address
#include <arpa/inet.h> //For getting the MAC address

ErrorType Wifi::init() {
    return ErrorType::NotAvailable;
}

ErrorType Wifi::networkUp() {
    return ErrorType::NotAvailable;
}

ErrorType Wifi::networkDown() {
    return ErrorType::NotAvailable;
}

ErrorType Wifi::transmit(const std::string &frame, const Socket socket, const Milliseconds timeout) {
    Bytes sent = 0;
    Bytes remaining = frame.size();

    assert(remaining > 0);

    while (remaining > 0) {
        ssize_t bytesWritten = send(socket, &frame.at(sent), remaining, 0);
        if (bytesWritten < 0) {
            return fromPlatformError(errno);
        }

        sent += bytesWritten;
        remaining -= bytesWritten;
    }

    return ErrorType::Success;
}

ErrorType Wifi::receive(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) {
    ErrorType error = ErrorType::Timeout;
    ssize_t bytesReceived = 0;

    Microseconds tvUsec = timeout * 1000;
    struct timeval timeoutval;
    if (tvUsec > std::numeric_limits<decltype(timeoutval.tv_usec)>::max()) {
        PLT_LOGW(TAG, "Truncating microseconds because it is bigger than the type used by this platform.");
        tvUsec = std::numeric_limits<decltype(timeoutval.tv_usec)>::max();
    }
    timeoutval.tv_sec = 0;
    timeoutval.tv_usec = static_cast<decltype(timeoutval.tv_usec)>(tvUsec);

    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);

    //Wait for input from the socket until the timeout
    {
    int ret;
    ret = select(socket + 1, &readfds, NULL, NULL, &timeoutval);
    if (ret < 0) {
        return fromPlatformError(errno);
    }
    }

    if (FD_ISSET(socket, &readfds)) {
        if (-1 == (bytesReceived = recv(socket, frameBuffer.data(), frameBuffer.size(), 0))) {
            error = fromPlatformError(errno);
        }
        else if (0 == bytesReceived) {
            //recv returns 0 if the connection is closed.
            error = ErrorType::PrerequisitesNotMet;
        }
        else {
            frameBuffer.resize(bytesReceived);
            error = ErrorType::Success;
        }
    }

    return error;
}

/*
 * In order to get the mac address, we use a common hostname like google and
 * run a test to see what interface is used when connecting to the common hostname.
 * The we use this hostname as our interface that we use to connect to the internet
 * and get the macAddress from that interface.
*/
ErrorType Wifi::getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) {
    std::array<char, IFNAMSIZ> interface;
    constexpr std::string_view host{"google.com"};
    std::array<char, NetworkTypes::Ipv4AddressStringSize> ipAddress;
    ErrorType error = ErrorType::Failure;

    if (ErrorType::Success == hostToIp(host, ipAddress)) {
        if (ErrorType::Success == interfaceRoutedTo(ipAddress, interface)) {
            int fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (fd >= 0) {
                struct ifreq ifr;
                std::memset(&ifr, 0, sizeof(ifr));
                std::strncpy(ifr.ifr_name, interface.data(), sizeof(ifr.ifr_name) - 1);
                if (0 == ioctl(fd, SIOCGIFHWADDR, &ifr)) {
                    unsigned char* hwaddr = reinterpret_cast<unsigned char*>(ifr.ifr_hwaddr.sa_data);
                    snprintf(macAddress.data(), macAddress.size(),
                        "%02x:%02x:%02x:%02x:%02x:%02x",
                        hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
                    error = ErrorType::Success;
                }

            }

            close(fd);
        }
    }

    return error;
}

ErrorType Wifi::getSignalStrength(DecibelMilliWatts &signalStrength) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::hostToIp(const std::string_view host, std::array<char, NetworkTypes::Ipv4AddressStringSize> &ipAddress) {
    struct addrinfo hints {};
    struct addrinfo* res = nullptr;
    ErrorType error = ErrorType::Failure;

    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(host.data(), nullptr, &hints, &res);
    if (ret == 0) {
        // Extract the IPv4 address
        struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
        const char* result = inet_ntop(AF_INET, &(addr->sin_addr), ipAddress.data(), ipAddress.size());
        freeaddrinfo(res);

        if (result == nullptr) {
            ipAddress[0] = '\0';
            error = ErrorType::Failure;
        }
        else {
            error = ErrorType::Success;
        }
    }

    return error;
}

ErrorType Wifi::interfaceRoutedTo(const std::array<char, NetworkTypes::Ipv4AddressStringSize> &ipAddress, std::array<char, IFNAMSIZ> &interface) {
    constexpr std::array<char, 128> command = {"sh -c \"ip route get %s\" | awk -F \"dev \" \'{print $2}\' | cut -d' ' -f1"};
    std::array<char, command.size() + NetworkTypes::Ipv4AddressStringSize> commandFinal;

    assert(snprintf(commandFinal.data(), commandFinal.size(), command.data(), ipAddress.data()) > 0);

    ErrorType error = ErrorType::Failure;
    
    FILE* pipe = popen(commandFinal.data(), "r");
    if (nullptr != pipe) {
        const size_t bytesRead = fread(interface.data(), sizeof(uint8_t), interface.max_size(), pipe);
        if (feof(pipe) || bytesRead == interface.max_size()) {
            error = ErrorType::Success;
            for (size_t i = 0; i < interface.size(); i++) {
                if (interface.at(i) == '\n') {
                    interface.at(i) = '\0';
                }
            }
        }
        else {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    return error;
}