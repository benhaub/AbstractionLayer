//AbstractionLayer
#include "HttpsClientModule.hpp"
//mbedtls
#include "mbedtls/error.h"
#include "mbedtls/x509_crt.h"

#if defined(__linux__)
#define CA_CERT "/etc/ssl/certs/ca-certificates.crt"
#elif defined(__APPLE__)
#define CA_CERT "/System/Library/Keychains/SystemRootCertificates.keychain"
#else
#error "Unsupported platform for CA_CERT" 
#endif

ErrorType HttpsClient::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &socket, const Milliseconds timeout) {
    assert(nullptr != _ipClient);
    ErrorType error = ErrorType::Failure;
    socket = -1;

    if (PSA_SUCCESS == psa_crypto_init()) {
        mbedtls_ctr_drbg_init(&_ctrDrbg);
        mbedtls_entropy_init(&_entropy);
        const char pers[] = "ssl_client1";

        if (0 == mbedtls_ctr_drbg_seed(&_ctrDrbg, mbedtls_entropy_func, &_entropy, (const unsigned char *) pers, sizeof(pers)-1)) {
            mbedtls_x509_crt_init(&_cacert);

            if (0 == mbedtls_x509_crt_parse_file(&_cacert, CA_CERT)) {
                mbedtls_net_init(&_serverFd);
                std::string portString = std::to_string(port);

                if (0 == mbedtls_net_connect(&_serverFd, hostname.data(), portString.c_str(), MBEDTLS_NET_PROTO_TCP)) {
                    mbedtls_ssl_config_init(&_conf);

                    if ((0 == mbedtls_ssl_config_defaults(&_conf,MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT))) {
                        mbedtls_ssl_conf_authmode(&_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
                        mbedtls_ssl_conf_ca_chain(&_conf, &_cacert, NULL);
                        mbedtls_ssl_init(&_ssl);

                        if ((0 == mbedtls_ssl_setup(&_ssl, &_conf))) {
                            if ((0 == mbedtls_ssl_set_hostname(&_ssl, hostname.data()))) {
                                mbedtls_ssl_set_bio(&_ssl, &_serverFd, mbedtls_net_send, mbedtls_net_recv, NULL);
                                
                                bool needToTryAgain = false;
                                bool handshakeFailed = false;
                                do {
                                    const int ret = mbedtls_ssl_handshake(&_ssl);
                                    needToTryAgain = (ret == MBEDTLS_ERR_SSL_WANT_READ ||
                                                        ret == MBEDTLS_ERR_SSL_WANT_WRITE ||
                                                        ret == MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS ||
                                                        ret == MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS);
                                    handshakeFailed = !needToTryAgain;
                                } while (needToTryAgain && !handshakeFailed);

                                const uint32_t flags = mbedtls_ssl_get_verify_result(&_ssl);
                                if (0 == flags) {
                                    error = ErrorType::Success;
                                    _isSslSession = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return error;
}

ErrorType HttpsClient::disconnect() {
    assert(nullptr != _ipClient);
    ErrorType error = ErrorType::Failure;

    if (0 == mbedtls_ssl_close_notify(&_ssl)) {
        error = ErrorType::Success;
    }

    return error;
}

ErrorType HttpsClient::sendBlocking(const HttpTypes::Request &request, const Milliseconds timeout) {
    assert(nullptr != _ipClient);
    ErrorType error = ErrorType::Success;
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
        frameNotFullyWritten = (frame.size() - written > 0);
                            
        if (noFatalErrorsOccured) {
            written += ret;
        }

    } while (needToTryAgain || (noFatalErrorsOccured && frameNotFullyWritten));

    return error;
}

ErrorType HttpsClient::receiveBlocking(HttpTypes::Response &response, const Milliseconds timeout) {
    assert(nullptr != _ipClient);
    assert(response.messageBody.size() > 0);
    ErrorType error = ErrorType::Success;
    Bytes read = 0;

    auto networkReceiveFunction = [&](std::string &buffer, const Milliseconds timeout) -> ErrorType {
        int ret = mbedtls_ssl_read(&_ssl, reinterpret_cast<uint8_t *>(&buffer[0]), buffer.size());
        if (ret < 0) {
            PLT_LOGW(TAG, "mbedtls_ssl_read failed to read response headers <error:-0x%x>", -ret);
            return ErrorType::Failure;
        }
        else {
            read = ret;
            return ErrorType::Success;
        }
    };

    if (0 == response.representationHeaders.contentLength) {
        error = readResponseHeaders(response, timeout, networkReceiveFunction);
    }
    else {
        int ret;

        do {
            ret = mbedtls_ssl_read(&_ssl, reinterpret_cast<uint8_t *>(&response.messageBody[read]), response.messageBody.size() - read);

            if (ret < 0) {
                PLT_LOGW(TAG, "mbedtls_ssl_read failed <error:-0x%x>", -ret);
                error = ErrorType::Failure;
            }
            else {
                read += ret;
            }


        } while (read < response.messageBody.size() && (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE));
    }

    if (read > 0 && error == ErrorType::Success) {
        response.messageBody.resize(read);
    }

    return error;
}

ErrorType HttpsClient::sendNonBlocking(const std::shared_ptr<HttpTypes::Request> request, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    assert(nullptr != _ipClient);

    return ErrorType::NotImplemented;
}

ErrorType HttpsClient::receiveNonBlocking(std::shared_ptr<HttpTypes::Response> request, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<HttpTypes::Request> buffer)> callback) {
    assert(nullptr != _ipClient);

    return ErrorType::NotImplemented;
}