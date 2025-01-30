//Modules
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
    if (-1 == send(socket, frame.data(), frame.size(), 0)) {
        return toPlatformError(errno);
    }

    return ErrorType::Success;
}

ErrorType Wifi::txNonBlocking(const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotAvailable;
}

//TODO: Needs to look like POSIX so that NoData is returned properly
ErrorType Wifi::rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;
    ssize_t bytesReceived = 0;

    struct timeval timeoutval = {
        .tv_sec = 0,
        .tv_usec = timeout * 1000
    };
    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);

    //Wait for input from the socket until the timeout
    {
    int ret;
    ret = select(socket + 1, &readfds, NULL, NULL, &timeoutval);
    if (ret < 0) {
        return toPlatformError(errno);
    }
    }

    if (FD_ISSET(socket, &readfds)) {
        if (-1 == (bytesReceived = recv(socket, frameBuffer.data(), frameBuffer.size(), 0))) {
            error = toPlatformError(errno);
        }
        else if ((size_t)bytesReceived > frameBuffer.size()) {
            error = ErrorType::PrerequisitesNotMet;
        }
        else {
            frameBuffer.resize(bytesReceived);
            return ErrorType::Success;
        }
    }
    else {
        error = ErrorType::Timeout;
    }

    frameBuffer.resize(0);

    return error;
}

ErrorType Wifi::rxNonBlocking(std::shared_ptr<std::string> frameBuffer, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> frameBuffer)> callback) {
    return ErrorType::NotAvailable;
}

ErrorType Wifi::getMacAddress(std::string &macAddress) {
    const Count macAddressSize = 17;
    //The command extracts just the mac address from the output.
    constexpr char command[] = "sh -c \"networksetup -getmacaddress Wi-Fi | cut -f 3 -d\' \' \"";
    ErrorType error = ErrorType::Failure;
    
    FILE* pipe = popen(command, "r");
    if (nullptr != pipe) {
        if (nullptr != fgets(macAddress.data(), macAddress.capacity(), pipe)) {
            macAddress.resize(macAddressSize);
            error = ErrorType::Success;
        }
        else {
            macAddress.clear();
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    return error;
}

ErrorType Wifi::getSignalStrength(DecibelMilliWatts &signalStrength) {
    std::string signalNoiseRatioString(64, 0);
    
    //The command extracts just the mac address from the output.
    //You can use the same command to get the noise as well to calculate a ratio .
    constexpr char command[] = "sh -c \"system_profiler -detailLevel basic SPAirPortDataType | egrep -A 10 'Current Network Information' | egrep 'Signal / Noise:' | cut -d: -f2 | tr -d [[^:numeric:]] | cut -d ' ' -f2\"";
    ErrorType error = ErrorType::Failure;
    
    FILE* pipe = popen(command, "r");
    if (nullptr != pipe) {
        if (nullptr != fgets(signalNoiseRatioString.data(), signalNoiseRatioString.capacity(), pipe)) {
            error = ErrorType::Success;
        }
        else {
            signalNoiseRatioString.clear();
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    sscanf(signalNoiseRatioString.data(), "%d", &signalStrength);

    _status.signalStrength = signalStrength;

    return error;
}

ErrorType Wifi::mainLoop() {
    return runNextEvent();
}