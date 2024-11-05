//AbstractionLayer
#include "NetworkAbstraction.hpp"
//AbstractionLayer modules
#include "IpClientModule.hpp"
#include "OperatingSystemModule.hpp"
//AbstractionLayer applications
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

ErrorType IpClient::connectTo(const std::string &hostname, const Port port, const IpClientSettings::Protocol protocol, const IpClientSettings::Version version, Socket &sock, const Milliseconds timeout) {
    auto connectCb = [=, this]() -> ErrorType {
        disconnect();

        if (version != IpClientSettings::Version::IPv4) {
            CBT_LOGE(TAG, "only IPv4 is implemented");
            return ErrorType::NotImplemented;
        }

        struct hostent *hent = gethostbyname(hostname.c_str());
        if (NULL == hent) {
            CBT_LOGW(TAG, "couldn't get address for %s", hostname.c_str());
            return ErrorType::Failure;
        }
        struct in_addr **addr_list = (struct in_addr **)hent->h_addr_list;
        struct sockaddr_in dest_ip;
        dest_ip.sin_addr.s_addr = addr_list[0]->s_addr;
        dest_ip.sin_family = toPosixFamily(version);
        dest_ip.sin_port = htons(port);

        if (-1 == (_socket = socket(toPosixFamily(version), toPosixSocktype(protocol), IPPROTO_IP))) {
        CBT_LOGW(TAG, "couldn't create socket");
        return ErrorType::Failure;
        }

        if (-1 == connect(_socket, (struct sockaddr *)&dest_ip, sizeof(dest_ip))) {
        CBT_LOGW(TAG, "couldn't connect to %s (%s)", hostname.c_str(), inet_ntoa(*(struct in_addr *)hent->h_addr_list[0]));
        close(_socket);
        return ErrorType::Failure;
        }

        CBT_LOGI(TAG, "connection in progress");
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(_socket, &fdset);

        // Connection in progress -> have to wait until the connecting socket is marked as writable, i.e. connection completes
        int res = select(_socket+1, NULL, &fdset, NULL, NULL);
        if (res < 0) {
            CBT_LOGW(TAG, "Error during connection: select for socket to be writable %s", strerror(errno));
            return ErrorType::Failure;
        } else if (res == 0) {
            CBT_LOGW(TAG, "Connection timeout: select for socket to be writable %s", strerror(errno));
            return ErrorType::Failure;
        } else {
            int sockerr;
            socklen_t len = (socklen_t)sizeof(int);

            if (getsockopt(_socket, SOL_SOCKET, SO_ERROR, (void*)(&sockerr), &len) < 0) {
                CBT_LOGW(TAG, "Error when getting socket error using getsockopt() %s", strerror(errno));
                return ErrorType::Failure;
            }
            if (sockerr) {
                CBT_LOGW(TAG, "Connection error %d", sockerr);
                return ErrorType::Failure;
            }
        }

        _status.connected = true;
        return ErrorType::Success;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<IpClient>>(std::bind(connectCb));
    ErrorType error;
    if (ErrorType::Success != (error = network().addEvent(event))) {
        CBT_LOGW(TAG, "Could not add connection event to network");
        return error;
    }

    Milliseconds i;
    for (i = 0; i < timeout / 10 && !_status.connected; i++) {
        OperatingSystem::Instance().delay(10);
    }

    if (!_status.connected && (timeout / 10) == i) {
        return ErrorType::Timeout;
    }
    else if (!_status.connected) {
        return ErrorType::Failure;
    }
    else {
        return ErrorType::Success;
    }
}

ErrorType IpClient::disconnect() {
    return ErrorType::NotImplemented;
}

ErrorType IpClient::sendBlocking(const std::string &data, const Milliseconds timeout) {
    assert(0 != _socket);
    bool sent = false;

    auto tx = [this, &sent](const std::string &frame, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        error = network().txBlocking(frame, _socket, timeout);
        if (ErrorType::Success != error) {
            _status.connected = false;
        }

        sent = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<IpClient>>(std::bind(tx, data, timeout));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    //Block for the timeout specified if no callback is provided
    Milliseconds i;
    for (i = 0; i < timeout / 10 && !sent; i++) {
        OperatingSystem::Instance().delay(10);
    }

    if (!sent && (timeout / 10) == i) {
        return ErrorType::Timeout;
    }
    else if (!sent) {
        return ErrorType::Failure;
    }
    else {
        return ErrorType::Success;
    }
}

ErrorType IpClient::receiveBlocking(std::string &buffer, const Milliseconds timeout) {
    bool received = false;

    auto rx = [this, &received](std::string *buffer, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        assert(0 != _socket);
        error = network().rxBlocking(*buffer, _socket, timeout);
        if (ErrorType::Success != error) {
            _status.connected = false;
        }

        received = true;
        return error;
    };

    //For some reason, I could pass buffer as a reference parameter to the callback. It had to be a pointer otherwise the
    //pointer to the data inside the string would change.
    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<IpClient>>(std::bind(rx, &buffer, timeout));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    Milliseconds i;
    for (i = 0; i < timeout / 10 && !received; i++) {
        OperatingSystem::Instance().delay(10);
    }

    if (!received && (timeout / 10) == i) {
        return ErrorType::Timeout;
    }
    else if (!received) {
        return ErrorType::Failure;
    }
    else {
        return ErrorType::Success;
    }
}

ErrorType IpClient::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    bool sent = false;

    auto tx = [this, callback, &sent](const std::shared_ptr<std::string> frame, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        if (nullptr == frame.get()) {
            return ErrorType::NoData;
        }

        error = network().txBlocking(*frame, _socket, timeout);

        assert(nullptr != callback);
        callback(error, frame->size());

        sent = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<IpClient>>(std::bind(tx, data, timeout));
    return network().addEvent(event);
}

ErrorType IpClient::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    assert(0 != _socket);
    bool received = false;

    auto rx = [this, callback, &received](const std::shared_ptr<std::string> buffer, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        if (nullptr == buffer.get()) {
            return ErrorType::NoData;
        }

        error = network().rxBlocking(*buffer, _socket, timeout);

        assert(nullptr != callback);
        callback(error, buffer);

        received = true;
        return error;
    };

    std::unique_ptr<EventAbstraction> event = std::make_unique<EventQueue::Event<IpClient>>(std::bind(rx, buffer, timeout));
    return network().addEvent(event);
}
