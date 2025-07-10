//AbstractionLayer
#include "HttpsClientModule.hpp"
//mbedtls
//#include "mbedtls/error.h"
//#include "mbedtls/x509_crt.h"
////ESP
//#include "esp_crt_bundle.h"
#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#ifdef CONFIG_MBEDTLS_SSL_PROTO_TLS1_3
#include "psa/crypto.h"
#endif
#include "esp_crt_bundle.h"
#include "OperatingSystemModule.hpp"

ErrorType HttpsClient::connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &socket, const Milliseconds timeout) {
    ErrorType error = ErrorType::Failure;
    socket = -1;

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

                                if ((0 == mbedtls_ssl_handshake(&_ssl))) {
                                    const int flags = mbedtls_ssl_get_verify_result(&_ssl);
                                    if (0 == flags) {
                                        error = ErrorType::Success;
                                    }
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

    response.messageBody.resize(read);

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