#ifndef __HTTPS_CLIENT_MODULE_HPP__
#define __HTTPS_CLIENT_MODULE_HPP__

//AbstractionLayer
#include "HttpClientAbstraction.hpp"
#include "IpClient.hpp"
//Esp
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/net_sockets.h"
#include "esp_netif.h"
#include "esp_event.h"

class HttpsClient final : public HttpClientAbstraction {

    public:
    HttpsClient() : HttpClientAbstraction() {
        esp_netif_init();
        esp_event_loop_create_default();
    }

    ~HttpsClient() {
        freeSslContexts();
    }

    ErrorType connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, const Milliseconds timeout) override;
    ErrorType disconnect() override;
    ErrorType sendBlocking(const HttpTypes::Request &request, const Milliseconds timeout) override;
    ErrorType receiveBlocking(HttpTypes::Response &response, const Milliseconds timeout) override;
    ErrorType sendNonBlocking(const std::shared_ptr<HttpTypes::Request> request, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveNonBlocking(std::shared_ptr<HttpTypes::Response> response, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<HttpTypes::Request> buffer)> callback) override;

    void setNetwork(NetworkAbstraction &network) override {
        HttpClientAbstraction::setNetwork(network);
        _ipClient.setNetwork(*_network);
    }

    private:
    IpClient _ipClient;

    bool _connected = false;
    mbedtls_ssl_context _ssl;
    mbedtls_entropy_context _entropy;
    mbedtls_ctr_drbg_context _ctrDrbg;
    mbedtls_ssl_config _conf;
    mbedtls_x509_crt _cacert;
    mbedtls_net_context _serverFd;

    void freeSslContexts() {
        mbedtls_ssl_free(&_ssl);
        mbedtls_entropy_free(&_entropy);
        mbedtls_ctr_drbg_free(&_ctrDrbg);
        mbedtls_ssl_config_free(&_conf);
        mbedtls_x509_crt_free(&_cacert);
        mbedtls_net_free(&_serverFd);
    }
};

#endif // __HTTPS_CLIENT_MODULE_HPP__