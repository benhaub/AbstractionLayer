//AbstractionLayer
#include "NetworkAbstraction.hpp"
#include "IpClientModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Log.hpp"
//Posix
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
//C++
#include <cassert>
#include <cstring>
#include <limits>

ErrorType IpClient::connectTo(std::string_view hostname, const Port port, const IpClientTypes::Protocol protocol, const IpClientTypes::Version version, Socket &sock, const Milliseconds timeout) {
    sock = -1;
    bool doneConnecting = false;
    ErrorType callbackError = ErrorType::Success;

    auto connectCb = [&](const Milliseconds timeout) -> ErrorType {
        disconnect();

        if (version != IpClientTypes::Version::IPv4) {
            PLT_LOGE(TAG, "only IPv4 is supported");
            callbackError = ErrorType::NotSupported;
            doneConnecting = true;
            _status.connected = false;
            return callbackError;
        }

        struct hostent *hent = gethostbyname(hostname.data());
        if (NULL == hent) {
            PLT_LOGW(TAG, "couldn't get address for %s", hostname.data());
            callbackError = ErrorType::Failure;
            doneConnecting = true;
            _status.connected = false;
            return callbackError;
        }
        struct in_addr **addr_list = (struct in_addr **)hent->h_addr_list;
        struct sockaddr_in dest_ip;
        dest_ip.sin_addr.s_addr = addr_list[0]->s_addr;
        dest_ip.sin_family = toPosixFamily(version);
        dest_ip.sin_port = htons(port);

        if (-1 == (_socket = socket(toPosixFamily(version), toPosixSocktype(protocol), IPPROTO_IP))) {
            PLT_LOGW(TAG, "couldn't create socket");
            callbackError = ErrorType::Failure;
            doneConnecting = true;
            _status.connected = false;
            return callbackError;
        }

        //If we get disconnected from the cloud, tell the OS not to signal us with SIGPIPE
        int one = 1;
        if (setsockopt(_socket, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one)) < 0) {
            PLT_LOGW(TAG, "couldn't set socket to not signal us with SIGPIPE");
            close(_socket);
            callbackError = ErrorType::Failure;
            doneConnecting = true;
            _status.connected = false;
            return callbackError;
        }

        if (-1 == connect(_socket, (struct sockaddr *)&dest_ip, sizeof(dest_ip))) {
            PLT_LOGW(TAG, "couldn't connect to %s (%s)", hostname.data(), inet_ntoa(*(struct in_addr *)hent->h_addr_list[0]));
            close(_socket);
            callbackError = ErrorType::Failure;
            doneConnecting = true;
            _status.connected = false;
            return callbackError;
        }

        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(_socket, &fdset);

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
        int res = select(_socket+1, NULL, &fdset, NULL, &timeoutval);
        if (res < 0) {
            PLT_LOGW(TAG, "Error during connection: select for socket to be writable %s", strerror(errno));
            callbackError = ErrorType::Failure;
            doneConnecting = true;
            _status.connected = false;
            return callbackError;
        }
        else if (res == 0) {
            PLT_LOGW(TAG, "Connection timeout: select for socket to be writable %s", strerror(errno));
            callbackError = ErrorType::Timeout;
            doneConnecting = true;
            _status.connected = false;
            return callbackError;
        }
        else {
            int sockerr;
            socklen_t len = (socklen_t)sizeof(int);

            if (getsockopt(_socket, SOL_SOCKET, SO_ERROR, (void*)(&sockerr), &len) < 0) {
                PLT_LOGW(TAG, "Error when getting socket error using getsockopt() %s", strerror(errno));
                callbackError = ErrorType::Failure;
                doneConnecting = true;
                _status.connected = false;
                return callbackError;
            }
            if (sockerr) {
                PLT_LOGW(TAG, "Connection error %d", sockerr);
                callbackError = ErrorType::Failure;
                doneConnecting = true;
                _status.connected = false;
                return callbackError;
            }
        }

        sock = _socket;
        _status.connected = true;
        doneConnecting = true;
        return callbackError;
    };

    ErrorType error = ErrorType::Failure;
    EventQueue::Event event = EventQueue::Event(std::bind(connectCb, timeout));
    if (ErrorType::Success != (error = network().addEvent(event))) {
        PLT_LOGW(TAG, "Could not add connection event to network");
        return error;
    }

    while (!doneConnecting) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpClient::disconnect() {
    if (_socket != -1) {
        shutdown(_socket, 0);
        close(_socket);
        _socket = -1;
    }

    return ErrorType::Success;
}

ErrorType IpClient::sendBlocking(const std::string &data, const Milliseconds timeout) {
    assert(0 != _socket);
    bool doneSending = false;
    ErrorType callbackError = ErrorType::Failure;

    auto tx = [&](const std::string &frame, const Milliseconds timeout) -> ErrorType {
        callbackError = network().txBlocking(frame, _socket, timeout);
        if (ErrorType::Success != callbackError) {
            _status.connected = false;
        }

        doneSending = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(tx, data, timeout));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!doneSending) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpClient::receiveBlocking(std::string &buffer, const Milliseconds timeout) {
    bool doneReceiving = false;
    ErrorType callbackError = ErrorType::Failure;

    auto rx = [&]() -> ErrorType {
        assert(0 != _socket);

        callbackError = network().rxBlocking(buffer, _socket, timeout);
        if (ErrorType::Success != callbackError) {
            _status.connected = false;
        }

        doneReceiving = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(rx));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    while (!doneReceiving) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return callbackError;
}

ErrorType IpClient::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    assert(0 != _socket);

    auto tx = [&, callback, data]() -> ErrorType {
        ErrorType error = ErrorType::Failure;

        assert(nullptr != callback);

        if (nullptr != data.get()) {
            error = network().txBlocking(*data, _socket, timeout);
            callback(error, data->size());
        }
        else {
            error = ErrorType::NoData;
            callback(error, 0);
        }

        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(tx));
    return network().addEvent(event);
}

ErrorType IpClient::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    assert(0 != _socket);

    auto rx = [&, callback, buffer]() -> ErrorType {
        ErrorType error = ErrorType::Failure;

        if (nullptr == buffer.get()) {
            return ErrorType::NoData;
        }

        error = network().rxBlocking(*buffer, _socket, timeout);

        assert(nullptr != callback);
        callback(error, buffer);

        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(rx));
    return network().addEvent(event);
}
