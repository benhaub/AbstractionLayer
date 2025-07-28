//AbstractionLayer
#include "WifiModule.hpp"
#include "Log.hpp"
//C++
#include <cstdio>
#include <cstring>
//Posix
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <net/if.h>
//Prevent the program from aborting if a connection is closed.
#include <signal.h>

ErrorType Wifi::init() {
    return ErrorType::NotAvailable;
}

ErrorType Wifi::networkUp() {
    return ErrorType::NotAvailable;
}

ErrorType Wifi::networkDown() {
    return ErrorType::NotAvailable;
}

ErrorType Wifi::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &sock, const Milliseconds timeout) {
    signal(SIGPIPE, SIG_IGN);
    ErrorType error = ErrorType::Failure;

    if (version == IpTypes::Version::IPv4) {
        struct addrinfo hints;
        struct addrinfo *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = toPosixFamily(version);
        hints.ai_socktype = toPosixSocktype(protocol);

        const int status = getaddrinfo(hostname.data(), std::to_string(port).c_str(), &hints, &res);
        if (0 == status) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
            struct sockaddr_in dest_ip;
            memcpy(&dest_ip, ipv4, sizeof(dest_ip));
            dest_ip.sin_port = htons(port);

            if (-1 != (sock = socket(toPosixFamily(version), toPosixSocktype(protocol), IPPROTO_IP))) {
                if (0 == connect(sock, (struct sockaddr *)&dest_ip, sizeof(dest_ip))) {
                    fd_set fdset;
                    FD_ZERO(&fdset);
                    FD_SET(sock, &fdset);

                    Microseconds tvUsec = timeout * 1000;
                    struct timeval timeoutval;
                    if (tvUsec > std::numeric_limits<decltype(timeoutval.tv_usec)>::max()) {
                        PLT_LOGW(TAG, "Truncating microseconds because it is bigger than the type used by this platform.");
                        tvUsec = std::numeric_limits<decltype(timeoutval.tv_usec)>::max();
                    }
                    //There is some a limit on the amount of usec's that can be used for Darwin but the limit is not stated, so try to use seconds if possible
                    if (timeout >= 1000) {
                        timeoutval.tv_sec = timeout / 1000;
                        timeoutval.tv_usec = 0;
                    }
                    else {
                        timeoutval.tv_sec = 0;
                        timeoutval.tv_usec = static_cast<decltype(timeoutval.tv_usec)>(tvUsec);
                    }

                    // Connection in progress -> have to wait until the connecting socket is marked as writable, i.e. connection completes
                    int res = select(sock+1, NULL, &fdset, NULL, &timeoutval);
                    if (res < 0) {
                        PLT_LOGW(TAG, "Error during connection: select for socket to be writable %s", strerror(errno));
                    }
                    else if (res == 0) {
                        PLT_LOGW(TAG, "Connection timeout: select for socket to be writable %s", strerror(errno));
                        error = ErrorType::Timeout;
                    }
                    else {
                        int sockerr;
                        socklen_t len = (socklen_t)sizeof(int);

                        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void*)(&sockerr), &len) < 0) {
                            PLT_LOGW(TAG, "Error when getting socket error using getsockopt() %s", strerror(errno));
                        }
                        else if (sockerr) {
                            PLT_LOGW(TAG, "Connection error %d", sockerr);
                        }
                        else {
                            error = ErrorType::Success;
                        }
                    }
                }
                else {
                    PLT_LOGW(TAG, "Failed to connect: %s", strerror(errno));
                    error = fromPlatformError(errno);
                }
            }
            else {
                PLT_LOGW(TAG, "Failed to create socket: %s", strerror(errno));
                error = fromPlatformError(errno);
            }

            freeaddrinfo(res);
        }
        else {
            PLT_LOGW(TAG, "Failed to get address info: %s", gai_strerror(status));
            error = ErrorType::PrerequisitesNotMet;
        }
    }
    else {
        error = ErrorType::NotSupported;
    }

    return error;
}

ErrorType Wifi::disconnect(const Socket &socket) {
    if (socket != -1) {
        shutdown(socket, 0);
        close(socket);
    }

    return ErrorType::Success;
}

ErrorType Wifi::listenTo(const IpTypes::Protocol protocol, const IpTypes::Version version, const Port port, Socket &listenerSocket) {
    struct addrinfo hints;
    struct addrinfo *servinfo = nullptr;
    struct addrinfo *p = nullptr;
    char portString[] = "65535";
    ErrorType error = ErrorType::Failure;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = toPosixFamily(version);
    hints.ai_socktype = toPosixSocktype(protocol);
    hints.ai_flags = AI_PASSIVE;

    assert(snprintf(portString, sizeof(portString), "%u", port) > 0);

    if (0 == getaddrinfo(nullptr, portString, &hints, &servinfo)) {
        for (p = servinfo; p != nullptr; p = p->ai_next) {
            if (-1 != (listenerSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol))) {
                int enable = 1;
                if (-1 != setsockopt(listenerSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable))) {
                    if (-1 != bind(listenerSocket, p->ai_addr, p->ai_addrlen)) {
                        //For more connections, create another instance of this class.
                        if (0 == listen(listenerSocket, 1)) {
                            //Socket is still invalid. The socket we just had is only for listening for connections.
                            //The socket we get from accept can be used to send and receive which is the one we want
                            //to return to the user.
                            return ErrorType::Success;
                        }
                    }
                }

                close(listenerSocket);
                error = fromPlatformError(errno);
                continue;
            }
        }

        freeaddrinfo(servinfo);
    }

    error = fromPlatformError(errno);
    return error;
}

ErrorType Wifi::acceptConnection(const Socket &listenerSocket, Socket &newSocket, const Milliseconds timeout) {
    struct sockaddr_storage clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    ErrorType error = ErrorType::Failure;

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
    FD_SET(listenerSocket, &readfds);

    int ret;
    ret = select(listenerSocket + 1, &readfds, NULL, NULL, &timeoutval);
    if (ret > 0) {
        if (FD_ISSET(listenerSocket, &readfds)) {
            if ((newSocket = accept(listenerSocket, (struct sockaddr *)&clientAddress, &clientAddressSize)) >= 0) {
                error = ErrorType::Success;
            }
            else {
                error = fromPlatformError(errno);
            }
        }
    }
    else if (0 == ret){
        error = ErrorType::Timeout;
    }
    else {
        error = fromPlatformError(errno);
    }

    return error;
}

ErrorType Wifi::closeConnection(const Socket socket) {
    if (-1 == close(socket)) {
        return fromPlatformError(errno);
    }
    else {
        return ErrorType::Success;
    }
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
    ErrorType error = ErrorType::Failure;
    ssize_t bytesReceived = 0;

    Microseconds tvUsec = timeout * 1000;
    struct timeval timeoutval;
    if (tvUsec > std::numeric_limits<decltype(timeoutval.tv_usec)>::max()) {
        PLT_LOGW(TAG, "Truncating microseconds because it is bigger than the type used by this platform.");
        tvUsec = std::numeric_limits<decltype(timeoutval.tv_usec)>::max();
    }
    //There is some a limit on the amount of usec's that can be used for Darwin but the limit is not stated, so try to use seconds if possible
    if (timeout >= 1000) {
        timeoutval.tv_sec = timeout / 1000;
        timeoutval.tv_usec = 0;
    }
    else {
        timeoutval.tv_sec = 0;
        timeoutval.tv_usec = static_cast<decltype(timeoutval.tv_usec)>(tvUsec);
    }

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

ErrorType Wifi::getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) {
    //The command extracts just the mac address from the output.
    constexpr char command[] = "sh -c \"networksetup -getmacaddress Wi-Fi | cut -f 3 -d\' \' \"";
    ErrorType error = ErrorType::Failure;
    
    FILE* pipe = popen(command, "r");
    if (nullptr != pipe) {
        const size_t bytesRead = fread(macAddress.data(), sizeof(uint8_t), macAddress.max_size(), pipe);
        if (feof(pipe) || bytesRead == macAddress.max_size()) {
            error = ErrorType::Success;
            for (size_t i = 0; i < bytesRead; i++) {
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
            for (size_t i = 0; i < bytesRead; i++) {
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