//Abstractions
#include "NetworkAbstraction.hpp"
//AbstractionLayer Modules
#include "IpClientModule.hpp"
#include "OperatingSystemModule.hpp"
#include "Log.hpp"
//Esp
#include "lwip/netdb.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
//C++
#include <cassert>
#include <cstring>
#include <limits>

/*
 * I tried the example code for non-blocking sockets from ESP github, it didn't work.
 * getaddrinfo would return a new IP address every time you call it and none of them
 * worked for DNS. If you didn't call gethostbyname (which getaddrinfo is meant
 * to replace) getaddrinfo would fail to find an IP every time you call it. Lastly, if you
 * call fcntl and set O_NONBLOCK on the socket it would fail to connect every time so there
 * was quite a symphony of bugs there.
*/
ErrorType IpClient::connectTo(std::string_view hostname, const Port port, const IpClientTypes::Protocol protocol, const IpClientTypes::Version version, Socket &sock, const Milliseconds timeout) {
    sock = -1;
    bool doneConnecting = false;
    ErrorType callbackError = ErrorType::Failure;

    auto connectCb = [&](const Milliseconds timeout) -> ErrorType {
        disconnect();

        if (version != IpClientTypes::Version::IPv4) {
            PLT_LOGE(TAG, "only IPv4 is supported");
            callbackError =  ErrorType::NotSupported;
            doneConnecting = true;
            _status.connected = false;
            return callbackError;
        }

        //While I think I have the code to do the rest of this in Ipv6, I don't know the code to do the DNS stuff in Ipv6.
        ip_addr_t ip_addr;
        ip_addr.type = IPADDR_TYPE_V4;
        uint8_t dns_server_ip[] = {8,8,8,8};
        IP4_ADDR(&ip_addr.u_addr.ip4, dns_server_ip[0], dns_server_ip[1], dns_server_ip[2], dns_server_ip[3]);
        dns_setserver(0, &ip_addr);
        struct hostent *hent = gethostbyname(hostname.data());
        if (NULL == hent) {
            PLT_LOGE(TAG, "couldn't get address for %s", hostname.data());
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
            PLT_LOGE(TAG, "couldn't create socket");
            callbackError = ErrorType::Failure;
            doneConnecting = true;
            _status.connected = false;
            return callbackError;
        }

        if (-1 == connect(_socket, (struct sockaddr *)&dest_ip, sizeof(dest_ip))) {
            PLT_LOGE(TAG, "couldn't connect to %s (%s)", hostname.data(), inet_ntoa(*(struct in_addr *)hent->h_addr_list[0]));
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
        //Try to use seconds if possible. Some systems don't like very large usec timeouts.
        if (timeout >= 1000) {
            timeoutval.tv_sec = timeout / 1000;
            timeoutval.tv_usec = 0;
        }
        else {
            timeoutval.tv_sec = 0;
            timeoutval.tv_usec = static_cast<decltype(timeoutval.tv_usec)>(tvUsec);
        }

        PLT_LOGI(TAG, "Connection in progress <timeout(ms): %u>", timeout);
        // Connection in progress -> have to wait until the connecting socket is marked as writable, i.e. connection completes
        int res = select(_socket+1, NULL, &fdset, NULL, &timeoutval);
        if (res < 0) {
            PLT_LOGE(TAG, "Error during connection: select for socket to be writable %s", strerror(errno));
            callbackError = ErrorType::Failure;
            doneConnecting = true;
            _status.connected = false;
            return callbackError;
        }
        else if (res == 0) {
            PLT_LOGE(TAG, "Connection timeout: select for socket to be writable %s", strerror(errno));
            callbackError = ErrorType::Timeout;
            doneConnecting = true;
            _status.connected = false;
            return callbackError;
        }
        else {
            int sockerr;
            socklen_t len = (socklen_t)sizeof(int);

            if (getsockopt(_socket, SOL_SOCKET, SO_ERROR, (void*)(&sockerr), &len) < 0) {
                PLT_LOGE(TAG, "Error when getting socket error using getsockopt() %s", strerror(errno));
                callbackError = ErrorType::Failure;
                doneConnecting = true;
                _status.connected = false;
                return callbackError;
            }
            if (sockerr) {
                PLT_LOGE(TAG, "Connection error %d", sockerr);
                callbackError = ErrorType::Failure;
                doneConnecting = true;
                _status.connected = false;
                return callbackError;
            }
        }

        sock = _socket;
        _status.connected = true;
        doneConnecting = true;
        return ErrorType::Success;
    };

    ErrorType error = ErrorType::Failure;
    EventQueue::Event event = EventQueue::Event(std::bind(connectCb, timeout));
    if (ErrorType::Success != (error = network().addEvent(event))) {
        return error;
    }

    while (!doneConnecting) {
        OperatingSystem::Instance().delay(Milliseconds(1));
    }

    return error;
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

    auto tx = [&]() -> ErrorType {
        callbackError = network().txBlocking(data, _socket, timeout);
        if (ErrorType::Success != callbackError) {
            _status.connected = false;
        }

        doneSending = true;
        return callbackError;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(tx));
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
    auto tx = [this, callback](const std::shared_ptr<std::string> frame, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        assert(nullptr != callback);

        if (nullptr == frame.get()) {
            callback(error, frame->size());
            return ErrorType::NoData;
        }

        error = network().txBlocking(*frame, _socket, timeout);

        callback(error, frame->size());

        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(tx, data, timeout));
    return network().addEvent(event);
}

ErrorType IpClient::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    assert(0 != _socket);

    auto rx = [this, callback](const std::shared_ptr<std::string> buffer, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        if (nullptr == buffer.get()) {
            callback(error, buffer);
            return ErrorType::NoData;
        }

        error = network().rxBlocking(*buffer, _socket, timeout);

        assert(nullptr != callback);
        callback(error, buffer);

        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(rx, buffer, timeout));
    return network().addEvent(event);
}
