//AbstractionLayer
#include "HttpsClientModule.hpp"
#include "OperatingSystemModule.hpp"
#include "NetworkAbstraction.hpp"
//mbedtls
#include "mbedtls/error.h"
#include "mbedtls/x509_crt.h"

#if defined(__linux__)
#define CA_CERT "/etc/ssl/certs/ca-certificates.crt"
#elif defined(__APPLE__)
#define CA_CERT "/private/etc/ssl/cert.pem"
#else
#error "Unsupported platform for CA_CERT" 
#endif

ErrorType HttpsClient::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout) {
    assert(nullptr != _network);
    ErrorType callbackError = ErrorType::Failure;
    volatile bool doneConnecting = false;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto connectCb = [&, thread]() -> ErrorType {
        disconnect();

        if (PSA_SUCCESS == psa_crypto_init()) {
            mbedtls_ctr_drbg_init(&_ctrDrbg);
            mbedtls_entropy_init(&_entropy);
            const char pers[] = "ssl_client1";

            if (0 == mbedtls_ctr_drbg_seed(&_ctrDrbg, mbedtls_entropy_func, &_entropy, (const unsigned char *) pers, sizeof(pers)-1)) {
                mbedtls_x509_crt_init(&_cacert);

                if (0 == mbedtls_x509_crt_parse_file(&_cacert, CA_CERT)) {

                    if (ErrorType::Success == (callbackError = _ipClient.connectTo(hostname, port, protocol, version, timeout))) {
                        _context.sock = _ipClient.sock();
                        mbedtls_ssl_config_init(&_conf);

                        if ((0 == mbedtls_ssl_config_defaults(&_conf,MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT))) {
                            mbedtls_ssl_conf_authmode(&_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
                            mbedtls_ssl_conf_ca_chain(&_conf, &_cacert, NULL);
                            mbedtls_ssl_init(&_ssl);

                            if ((0 == mbedtls_ssl_setup(&_ssl, &_conf))) {

                                if ((0 == mbedtls_ssl_set_hostname(&_ssl, hostname.data()))) {
                                    mbedtls_ssl_set_user_data_p(&_ssl, &_ipClient.network());
                                    _context.sslContext = &_ssl;
                                    mbedtls_ssl_set_bio(&_ssl, &_context, MbedTlsCompatible::Send, NULL, MbedTlsCompatible::Receive);
                                    mbedtls_ssl_conf_read_timeout(&_conf, timeout);
                                    
                                    bool needToTryAgain = false;
                                    bool handshakeFailed = false;

                                    do {
                                        const int ret = mbedtls_ssl_handshake(&_ssl);
                                        needToTryAgain = (ret == MBEDTLS_ERR_SSL_WANT_READ ||
                                                            ret == MBEDTLS_ERR_SSL_WANT_WRITE ||
                                                            ret == MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS ||
                                                            ret == MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS);
                                        handshakeFailed = !needToTryAgain && ret < 0;
                                    } while (needToTryAgain && !handshakeFailed);

                                    const uint32_t flags = mbedtls_ssl_get_verify_result(&_ssl);

                                    if (0 != flags || handshakeFailed) {
                                        callbackError = ErrorType::Failure;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        callbackError == ErrorType::Success ? _connected = true : _connected = false;
        doneConnecting = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    ErrorType error = ErrorType::Failure;
    EventQueue::Event event = EventQueue::Event(std::bind(connectCb));
    if (ErrorType::Success != (error = _ipClient.network().addEvent(event))) {
        return error;
    }

    while (!doneConnecting && ErrorType::LimitReached == OperatingSystem::Instance().block());

    return callbackError;
}

ErrorType HttpsClient::disconnect() {
    assert(nullptr != _network);

    if (_connected) {
        ErrorType callbackError = ErrorType::Failure;
        volatile bool doneDisconnecting = false;
        Id thread;
        OperatingSystem::Instance().currentThreadId(thread);

        auto disconnectCb = [&, thread]() -> ErrorType {
            if (0 == mbedtls_ssl_close_notify(&_ssl)) {
                callbackError = ErrorType::Success;
            }

            mbedtls_ssl_session_reset(&_ssl);
            freeSslContexts();

            _connected = false;
            OperatingSystem::Instance().unblock(thread);
            doneDisconnecting = true;
            return callbackError;
        };

        ErrorType error = ErrorType::Failure;
        EventQueue::Event event = EventQueue::Event(disconnectCb);
        if (ErrorType::Success != (error = _ipClient.network().addEvent(event))) {
            return error;
        }

        while (!doneDisconnecting && ErrorType::LimitReached == OperatingSystem::Instance().block());

        return callbackError;
    }
    else {
        return ErrorType::Success;
    }
}

ErrorType HttpsClient::sendBlocking(const HttpTypes::Request &request, const Milliseconds timeout) {
    assert(nullptr != _network);
    ErrorType callbackError = ErrorType::Failure;;
    volatile bool doneSending = false;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto sendCb = [&, thread]() -> ErrorType {
        //Big enough that hopefully the string doesn't have to reallocate.
        constexpr Bytes headerSize = 512;
        std::string frame(headerSize + request.messageBody.size(), 0);

        const Bytes initialCapacity = frame.capacity();
        HttpTypes::FromHttpRequest(request, frame);

        if (initialCapacity > frame.capacity()) {
            PLT_LOGW(TAG, "frame size had to be increased from %u to %u", initialCapacity, frame.capacity());
        }

        int ret;
        bool needToTryAgain = true;
        bool noFatalErrorsOccured = false;
        bool frameNotFullyWritten = false;
        Bytes written = 0;

        do {
            ret = mbedtls_ssl_write(&_ssl, reinterpret_cast<uint8_t *>(&frame[written]), frame.size() - written);
            needToTryAgain = (ret == MBEDTLS_ERR_SSL_WANT_READ ||
                            ret == MBEDTLS_ERR_SSL_WANT_WRITE ||
                            ret == MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS ||
                            ret == MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS);
                                
            noFatalErrorsOccured = ret >= 0;

            if (noFatalErrorsOccured) {
                written += ret;
            }
            else {
                PLT_LOGW(TAG, "mbedtls_ssl_write failed <error:-0x%x>", -ret);
            }

            frameNotFullyWritten = (frame.size() - written > 0);

        } while (needToTryAgain || (noFatalErrorsOccured && frameNotFullyWritten));

        noFatalErrorsOccured ? callbackError = ErrorType::Success : callbackError = ErrorType::Failure;
        OperatingSystem::Instance().unblock(thread);
        doneSending = true;
        return callbackError;
    };

    ErrorType error = ErrorType::Failure;
    EventQueue::Event event = EventQueue::Event(sendCb);
    if (ErrorType::Success != (error = _ipClient.network().addEvent(event))) {
        return error;
    }

    while (!doneSending && ErrorType::LimitReached == OperatingSystem::Instance().block());

    return callbackError;
}

ErrorType HttpsClient::receiveBlocking(HttpTypes::Response &response, const Milliseconds timeout) {
    assert(nullptr != _network);
    const Bytes messageBodySize = response.messageBody.size();
    assert(messageBodySize > 0);
    ErrorType callbackError = ErrorType::Success;
    Bytes read = 0;
    volatile bool doneReceiving = false;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto receiveCallback = [&, thread]() -> ErrorType {
        auto networkReceiveFunction = [&](std::string &buffer, const Milliseconds timeout) -> ErrorType {
            mbedtls_ssl_conf_read_timeout(&_conf, timeout);
            int ret = mbedtls_ssl_read(&_ssl, reinterpret_cast<uint8_t *>(&buffer[0]), buffer.size());

            if (ret < 0) {
                if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
                    // Need more data or write buffer full - retry
                    return ErrorType::Success; // Indicate we need to retry
                }
                if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
                    // Server closed connection gracefully - this is normal
                    return ErrorType::Success;
                }
                if (ret == MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET) {
                    // Server sent new session ticket - continue operation
                    return ErrorType::Success; // Indicate we need to retry
                }

                PLT_LOGW(TAG, "mbedtls_ssl_read failed to read response headers <error:-0x%x>", -ret);
                return ErrorType::Failure;
            }
            else {
                buffer.resize(ret);
                return ErrorType::Success;
            }
        };

        if (0 == response.representationHeaders.contentLength) {

            do {
                response.messageBody.resize(messageBodySize);

                callbackError = networkReceiveFunction(response.messageBody, timeout);

                if (ErrorType::Success == callbackError) {
                    callbackError = readResponseHeaders(response, timeout);
                }
            } while (ErrorType::Negative == callbackError);

            read = response.messageBody.size();

            if (0 == read) {
                //If exactly the header was read, then no bytes from the message body were read so we can resize to start reading it.
                response.messageBody.resize(messageBodySize);
            }
        }

        int ret;

        do {
            ret = mbedtls_ssl_read(&_ssl, reinterpret_cast<uint8_t *>(&response.messageBody[read]), response.messageBody.size() - read);

            if (ret < 0) {

                if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
                    // Need more data or write buffer full - continue loop
                    continue;
                }
                if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
                    // Server closed connection gracefully - this is normal
                    break;
                }
                if (ret == MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET) {
                    // Server sent new session ticket - continue operation
                    continue;
                }

                PLT_LOGW(TAG, "mbedtls_ssl_read failed <error:-0x%x>", -ret);
                callbackError = ErrorType::Failure;
            }
            else {
                read += ret;
            }


        } while (read < response.messageBody.size() && (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) && ErrorType::Success == callbackError);

        if (read > 0 && callbackError == ErrorType::Success) {
            response.messageBody.resize(read);
        }

        doneReceiving = true;
        OperatingSystem::Instance().unblock(thread);
        return callbackError;
    };

    ErrorType error = ErrorType::Failure;
    EventQueue::Event event = EventQueue::Event(receiveCallback);
    if (ErrorType::Success != (error = _ipClient.network().addEvent(event))) {
        return error;
    }

    while (!doneReceiving && ErrorType::LimitReached == OperatingSystem::Instance().block());

    return callbackError;
}

ErrorType HttpsClient::sendNonBlocking(const std::shared_ptr<HttpTypes::Request> request, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    assert(nullptr != _network);

    return ErrorType::NotImplemented;
}

ErrorType HttpsClient::receiveNonBlocking(std::shared_ptr<HttpTypes::Response> request, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<HttpTypes::Request> buffer)> callback) {
    assert(nullptr != _network);

    return ErrorType::NotImplemented;
}