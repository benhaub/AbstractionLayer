#ifndef __MBEDTLS_SEND_RECEIVE_HPP__
#define __MBEDTLS_SEND_RECEIVE_HPP__

//AbstractionLayer
#include "Types.hpp"
//C++
#include <cstddef>
//MbedTls
#include "mbedtls/ssl.h"

/**
 * @namespace MbedTlsCompatible
 * @brief MbedTLS compatible send and receive functions for the AbstractionLayer Network.
 */
namespace MbedTlsCompatible {
    /**
     * @struct BioContext
     * @brief Parameter that is shared between the send and receive functions.
     * @sa mbedtls_ssl_set_bio
     */
    struct BioContext {
        Socket sock;                     ///< The socket that data is being sent or received on.
        mbedtls_ssl_context *sslContext; ///< The SSL context that is being used.
    };

    int Send(void *ctx, const unsigned char *buf, size_t len);
    int Receive(void *ctx, unsigned char *buf, size_t len);
}

#endif // __MBEDTLS_SEND_RECEIVE_HPP__