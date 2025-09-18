//AbstractionLayer
#include "HttpsClientModule.hpp"
#include "OperatingSystemModule.hpp"
#include "NetworkAbstraction.hpp"
//MBedTLS
#include "psa/crypto.h"
//ESP
#include "esp_crt_bundle.h"

ErrorType HttpsClient::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout) {
    assert(nullptr != _network);
    ErrorType callbackError = ErrorType::Failure;
    bool doneConnecting = false;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto connectCb = [&, thread]() -> ErrorType {
        disconnect();

        if (PSA_SUCCESS == psa_crypto_init()) {
            mbedtls_ctr_drbg_init(&_ctrDrbg);
            mbedtls_entropy_init(&_entropy);

            if(0 == mbedtls_ctr_drbg_seed(&_ctrDrbg, mbedtls_entropy_func, &_entropy, NULL, 0)) {
                mbedtls_ssl_init(&_ssl);
                mbedtls_x509_crt_init(&_cacert);
                mbedtls_ssl_config_init(&_conf);

                if (ESP_OK == esp_crt_bundle_attach(&_conf)) {
                    /* Hostname set here should match CN in server certificate */
                    if (0 == mbedtls_ssl_set_hostname(&_ssl, hostname.data())) {
                        const int endpoint = MBEDTLS_SSL_IS_CLIENT;
                        const int transport = MBEDTLS_SSL_TRANSPORT_STREAM;
                        const int preset = MBEDTLS_SSL_PRESET_DEFAULT;

                        if (0 == mbedtls_ssl_config_defaults(&_conf, endpoint, transport, preset)) {
                            mbedtls_ssl_conf_authmode(&_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
                            mbedtls_ssl_conf_ca_chain(&_conf, &_cacert, NULL);
                            mbedtls_ssl_conf_rng(&_conf, mbedtls_ctr_drbg_random, &_ctrDrbg);

                            if (0 == mbedtls_ssl_setup(&_ssl, &_conf)) {
                                mbedtls_net_init(&_serverFd);

                                if (0 == mbedtls_net_connect(&_serverFd, hostname.data(), std::to_string(port).c_str(), MBEDTLS_NET_PROTO_TCP)) {
                                    mbedtls_ssl_set_bio(&_ssl, &_serverFd, mbedtls_net_send, mbedtls_net_recv, NULL);

                                    int ret;
                                    while ((ret = mbedtls_ssl_handshake(&_ssl)) != 0) {
                                        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
                                            PLT_LOGW(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
                                            doneConnecting = true;
                                            return callbackError;
                                        }
                                    }

                                    callbackError = ErrorType::Success;
                                }
                            }
                        }
                    }
                }
            }
        }

        callbackError == ErrorType::Success ? _connected = true : _connected = false;
        OperatingSystem::Instance().unblock(thread);
        doneConnecting = true;
        return callbackError;
    };

    ErrorType error = ErrorType::Failure;
    EventQueue::Event event = EventQueue::Event(connectCb);
    if (ErrorType::Success != (error = _ipClient.network().addEvent(event))) {
        return error;
    }

    while (!doneConnecting) {
        OperatingSystem::Instance().block();
    }

    return callbackError;
}

ErrorType HttpsClient::disconnect() {
    assert(nullptr != _network);

    if (_connected) {
        ErrorType callbackError = ErrorType::Failure;
        bool doneDisconnecting = false;
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

        while (!doneDisconnecting) {
            OperatingSystem::Instance().block();
        }

        return callbackError;
    }
    else {
        return ErrorType::Success;
    }
}

ErrorType HttpsClient::sendBlocking(const HttpTypes::Request &request, const Milliseconds timeout) {
    assert(nullptr != _network);
    ErrorType callbackError = ErrorType::Failure;;
    bool doneSending = false;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto sendCb = [&, thread]() -> ErrorType {
        //Big enough that hopefully the string doesn't have to reallocate.
        constexpr Bytes headerSize = 512;
        std::string frame(headerSize + request.messageBody.size(), 0);

        const Bytes initialCapacity = frame.capacity();
        HttpTypes::fromHttpRequest(request, frame);

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
 
            noFatalErrorsOccured = ret > 0;

            if (noFatalErrorsOccured) {
                written += ret;
            }
            else if (ret == 0) {
                PLT_LOGW(TAG, "mbedtls_ssl_write sent 0 bytes <frameSize:%u, written:%u>", frame.size(), written);
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

    while (!doneSending) {
        OperatingSystem::Instance().block();
    }

    return callbackError;
}

ErrorType HttpsClient::receiveBlocking(HttpTypes::Response &response, const Milliseconds timeout) {
    assert(nullptr != _network);
    const Bytes messageBodySize = response.messageBody.size();
    assert(messageBodySize > 0);
    ErrorType callbackError = ErrorType::Success;
    Bytes read = 0;
    bool doneReceiving = false;
    Id thread;
    OperatingSystem::Instance().currentThreadId(thread);

    auto receiveCallback = [&, thread]() -> ErrorType {
        auto networkReceiveFunction = [&](std::string &buffer, const Milliseconds timeout) -> ErrorType {
            int ret = mbedtls_ssl_read(&_ssl, reinterpret_cast<uint8_t *>(&buffer[0]), buffer.size());
            if (ret < 0) {
                if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
                    // Need more data or write buffer full - retry
                    buffer.resize(0);
                    return ErrorType::Success; // Indicate we need to retry
                }
                if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
                    // Server closed connection gracefully - this is normal
                    buffer.resize(0);
                    return ErrorType::Success;
                }
                if (ret == MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET) {
                    // Server sent new session ticket - continue operation
                    buffer.resize(0);
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
        }
        else {
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


            } while (read < response.messageBody.size() && (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE));
        }

        if (read > 0 && callbackError == ErrorType::Success) {
            response.messageBody.resize(read);
        }

        OperatingSystem::Instance().unblock(thread);
        doneReceiving = true;
        return callbackError;
    };

    ErrorType error = ErrorType::Failure;
    EventQueue::Event event = EventQueue::Event(receiveCallback);
    if (ErrorType::Success != (error = _ipClient.network().addEvent(event))) {
        return error;
    }

    while (!doneReceiving) {
        OperatingSystem::Instance().block();
    }

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