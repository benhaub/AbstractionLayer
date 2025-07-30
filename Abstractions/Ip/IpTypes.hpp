#ifndef __IP_TYPES_HPP__
#define __IP_TYPES_HPP__

#include <cstdint>

/**
 * @namespace IpTypes
 * @brief Types for IP connections
*/
namespace IpTypes {

    /**
     * @enum Version
     * @brief The version to use for the IP connection
    */
    enum class Version : uint8_t {
        Unknown = 0, ///< Unknown
        IPv4,        ///< Internet Protocol Version 4
        IPv6,        ///< Internet Protocol Version 6
        IPv4v6,      ///< Internet Protocol Version 4 and 6
    };

    /**
     * @enum Protocol
     * @brief The protocol to use for the IP connection
    */
    enum class Protocol : uint8_t {
        Unknown = 0, ///< Unknown
        Tcp,         ///< Transmission Control Protocol
        Udp          ///< User Datagram Protocol
    };
}

#endif //__IP_TYPES_HPP__