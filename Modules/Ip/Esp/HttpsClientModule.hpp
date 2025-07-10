#ifndef __HTTP_CLIENT_MODULE_HPP__
#define __HTTP_CLIENT_MODULE_HPP__

//AbstractionLayer
#include "HttpClientAbstraction.hpp"
//MbedTLS
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/net_sockets.h"

class HttpsClient final : public HttpClientAbstraction {

    public:
    HttpsClient(IpClientAbstraction &ipClient) : HttpClientAbstraction(ipClient) {}

    ErrorType connectTo(std::string_view hostname, const Port port, const IpTypes::Protocol protocol, const IpTypes::Version version, Socket &socket, const Milliseconds timeout) override;
    ErrorType disconnect() override;
    ErrorType sendBlocking(const HttpTypes::Request &request, const Milliseconds timeout) override;
    ErrorType receiveBlocking(HttpTypes::Response &response, const Milliseconds timeout) override;
    ErrorType sendNonBlocking(const std::shared_ptr<HttpTypes::Request> request, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType receiveNonBlocking(std::shared_ptr<HttpTypes::Response> response, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<HttpTypes::Request> buffer)> callback) override;

    private:
    bool _isSslSession = false;
    mbedtls_ssl_context _ssl;
    mbedtls_entropy_context _entropy;
    mbedtls_ctr_drbg_context _ctrDrbg;
    mbedtls_ssl_config _conf;
    mbedtls_x509_crt _cacert;
    mbedtls_net_context _serverFd;
};

#endif // __HTTP_CLIENT_MODULE_HPP__