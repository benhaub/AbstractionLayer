//AbstractionLayer
#include "WifiModule.hpp"
//C++
#include <cassert>
#include <cstdio>
//Posix
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

ErrorType Wifi::init() {
    return ErrorType::NotAvailable;
}

ErrorType Wifi::networkUp() {
    return ErrorType::NotAvailable;
}

ErrorType Wifi::networkDown() {
    return ErrorType::NotAvailable;
}

ErrorType Wifi::txBlocking(const std::string &frame, const Socket socket, const Milliseconds timeout) {
    Bytes sent = 0;
    Bytes remaining = frame.size();

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

ErrorType Wifi::txNonBlocking(const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotAvailable;
}

ErrorType Wifi::rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) {
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
        frameBuffer.resize(0);
        return fromPlatformError(errno);
    }
    }

    if (FD_ISSET(socket, &readfds)) {
        if (-1 == (bytesReceived = recv(socket, frameBuffer.data(), frameBuffer.size(), 0))) {
            frameBuffer.resize(0);
            error = fromPlatformError(errno);
        }
        else if (0 == bytesReceived) {
            //recv returns 0 if the connection is closed.
            frameBuffer.resize(0);
            error = ErrorType::PrerequisitesNotMet;
        }
        else {
            frameBuffer.resize(bytesReceived);
            error = ErrorType::Success;
        }
    }

    return error;
}

ErrorType Wifi::rxNonBlocking(std::shared_ptr<std::string> frameBuffer, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> frameBuffer)> callback) {
    return ErrorType::NotAvailable;
}

/*
 * In order to get the mac address, we use a common hostname like google and
 * run a test to see what interface is used when connecting to the common hostname.
 * The we use this hostname as our interface that we use to connect to the internet
 * and get the macAddress from that interface.
*/
ErrorType Wifi::getMacAddress(std::string &macAddress) {
    std::string interface(16, 0);
    std::string host("google.com");
    std::string ipAddress("127.127.127.127");

    hostToIp(host, ipAddress);
    interfaceRoutedTo(ipAddress, interface);

    const std::string command1("sh -c \"ip addr show ");
    const std::string command2("\" | egrep \"link/ether\" | tr -s ' ' | cut -d' ' -f3");
    std::string commandFinal(command1.size() + command2.size() + macAddress.size(), 0);
    commandFinal.assign(command1);
    commandFinal.append(interface);
    commandFinal.append(command2);

    ErrorType error = ErrorType::Failure;
    
    FILE* pipe = popen(commandFinal.c_str(), "r");
    if (nullptr != pipe) {
        size_t bytesRead = fread(macAddress.data(), sizeof(uint8_t), macAddress.capacity(), pipe);
        if (feof(pipe) || bytesRead == macAddress.capacity()) {
            error = ErrorType::Success;
            macAddress.resize(bytesRead);
            while (macAddress.back() == '\n') {
                macAddress.pop_back();
            }
        }
        else if (ferror(pipe)) {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    return error;
}

ErrorType Wifi::getSignalStrength(DecibelMilliWatts &signalStrength) {
    return ErrorType::NotImplemented;
}

ErrorType Wifi::hostToIp(const std::string &host, std::string &ipAddress) {
    const std::string command1("sh -c \"getent ahosts ");
    const std::string command2(" | awk '{print $1; exit}' | cut -d' ' -f1\"");
    std::string commandFinal(command1.size() + command2.size() + ipAddress.size(), 0);
    commandFinal.assign(command1);
    commandFinal.append(host);
    commandFinal.append(command2);

    ErrorType error = ErrorType::Failure;
    
    FILE* pipe = popen(commandFinal.c_str(), "r");
    if (nullptr != pipe) {
        const size_t bytesRead = fread(ipAddress.data(), sizeof(uint8_t), ipAddress.capacity(), pipe);
        if (feof(pipe) || bytesRead == ipAddress.capacity()) {
            error = ErrorType::Success;
            ipAddress.resize(bytesRead);
            while (ipAddress.back() == '\n') {
                ipAddress.pop_back();
            }
        }
        else if (ferror(pipe)) {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    return error;
}

ErrorType Wifi::interfaceRoutedTo(const std::string &ipAddress, std::string &interface) {
    const std::string command1("sh -c \"ip route get ");
    const std::string command2("\" | awk -F \"dev \" \'{print $2}\' | cut -d' ' -f1");
    std::string commandFinal(command1.size() + command2.size() + ipAddress.size(), 0);
    commandFinal.assign(command1);
    commandFinal.append(ipAddress);
    commandFinal.append(command2);

    ErrorType error = ErrorType::Failure;
    
    FILE* pipe = popen(commandFinal.c_str(), "r");
    if (nullptr != pipe) {
        const size_t bytesRead = fread(interface.data(), sizeof(uint8_t), interface.capacity(), pipe);
        if (feof(pipe) || bytesRead == interface.capacity()) {
            error = ErrorType::Success;
            interface.resize(bytesRead);
            while (interface.back() == '\n') {
                interface.pop_back();
            }
        }
        else {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    return error;
}

ErrorType Wifi::mainLoop() {
    return runNextEvent();
}