//AbstractionLayer
#include "WifiModule.hpp"
#include "Log.hpp"
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

//TODO: Needs to look like POSIX so that NoData is returned properly
ErrorType Wifi::rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;
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

ErrorType Wifi::getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) {
    //The command extracts just the mac address from the output.
    constexpr char command[] = "sh -c \"networksetup -getmacaddress Wi-Fi | cut -f 3 -d\' \' \"";
    ErrorType error = ErrorType::Failure;
    
    FILE* pipe = popen(command, "r");
    if (nullptr != pipe) {
        const size_t bytesRead = fread(macAddress.data(), sizeof(uint8_t), macAddress.max_size(), pipe);
        if (feof(pipe) || bytesRead == macAddress.max_size()) {
            error = ErrorType::Success;
            for (size_t i = 0; i < macAddress.size(); i++) {
                if (macAddress.at(i) == '\n') {
                    macAddress.at(i) = '\0';
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

ErrorType Wifi::getSignalStrength(DecibelMilliWatts &signalStrength) {
    std::array<char, 64> signalNoiseRatioString;
    
    //The command extracts just the mac address from the output.
    //You can use the same command to get the noise as well to calculate a ratio .
    constexpr char command[] = "sh -c \"system_profiler -detailLevel basic SPAirPortDataType | egrep -A 10 'Current Network Information' | egrep 'Signal / Noise:' | cut -d: -f2 | tr -d [[^:numeric:]] | cut -d ' ' -f2\"";
    ErrorType error = ErrorType::Failure;
    
    FILE* pipe = popen(command, "r");
    if (nullptr != pipe) {
        const size_t bytesRead = fread(signalNoiseRatioString.data(), sizeof(uint8_t), signalNoiseRatioString.max_size(), pipe);
        if (feof(pipe) || bytesRead == signalNoiseRatioString.max_size()) {
            error = ErrorType::Success;
            for (size_t i = 0; i < signalNoiseRatioString.size(); i++) {
                if (signalNoiseRatioString.at(i) == '\n') {
                    signalNoiseRatioString.at(i) = '\0';
                }
            }
        }
        else {
            pclose(pipe);
            error = ErrorType::Failure;
        }
    }

    sscanf(signalNoiseRatioString.data(), "%hd", &signalStrength);

    _status.signalStrength = signalStrength;

    return error;
}

ErrorType Wifi::mainLoop() {
    return runNextEvent();
}