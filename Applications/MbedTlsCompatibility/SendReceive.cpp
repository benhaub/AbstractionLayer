//AbstracionLayer
#include "SendReceive.hpp"
#include "NetworkAbstraction.hpp"
//MbedTls
#include "mbedtls/net_sockets.h"

namespace MbedTlsCompatible {

    int Send(void *ctx, const unsigned char *buf, size_t len) {
        BioContext *context = ((BioContext *) ctx);
        assert(context != nullptr);
        NetworkAbstraction *network = reinterpret_cast<NetworkAbstraction *>(mbedtls_ssl_get_user_data_p(context->sslContext));
        assert(network != nullptr);

        const char *buffer = reinterpret_cast<const char *>(buf);
        const ErrorType error = network->transmit(std::string_view(buffer, len), context->sock, Milliseconds(0));

        if (ErrorType::Timeout == error) {
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        }
        else if (ErrorType::Success == error) {
            return len;
        }
        else {
            return MBEDTLS_ERR_NET_SEND_FAILED;
        }
    }

    int Receive(void *ctx, unsigned char *buf, size_t len, uint32_t timeout) {
        BioContext *context = ((BioContext *) ctx);
        assert(context != nullptr);
        NetworkAbstraction *network = reinterpret_cast<NetworkAbstraction *>(mbedtls_ssl_get_user_data_p(context->sslContext));
        assert(network != nullptr);

        char *buffer = reinterpret_cast<char *>(buf);
        Bytes received = 0;
        const ErrorType error = network->receive(buffer, len, context->sock, received, Milliseconds(timeout));

        if (ErrorType::Timeout == error) {
            return MBEDTLS_ERR_SSL_WANT_READ;
        }
        else if (ErrorType::Success == error) {
            return received;
        }
        else {
            return MBEDTLS_ERR_NET_RECV_FAILED;
        }
    }
}