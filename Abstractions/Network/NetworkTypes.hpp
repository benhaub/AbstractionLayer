/***************************************************************************//**
* @author  Ben Haubrich
* @file    NetworkTypes.hpp
* @details Interface for communication over the network.
* @ingroup Abstractions
*******************************************************************************/
#ifndef __NETWORK_TYPES_HPP__
#define __NETWORK_TYPES_HPP__

//AbstractionLayer
#include "Math.hpp"
#include "Types.hpp"

/**
 * @namespace NetworkTypes
 * @brief Namespace for types related to the network interface.
 */
namespace NetworkTypes {
    
    /**
     * @enum Technology
     * @brief The technology of the network interface.
     * @note If you google wifi, 3g, or 4g, It will say they are all wireless communication technologies.
     *       That's where the name comes from.
    */
    enum class Technology {
        Unknown = 0, ///< Unknown
        Wifi,        ///< Wi-Fi
        Zigbee,      ///< ZigBee
        Ethernet,    ///< Ethernet
        Cellular,    ///< Cellular
    };

    /**
     * @struct Status
     * @brief The status of the network interface.
    */
    struct Status {
        bool isUp;             ///< True when the network is up and ready for use.
        Technology technology; ///< The technology of the network interface.

        Status() : isUp(false), technology(Technology::Unknown) {}
    };

    /// @brief Size of an IPv4 address string with tombstone
    constexpr uint8_t Ipv4AddressStringSize = 18;
    /// @brief Size of a MAC address string with tombstone
    constexpr uint8_t MacAddressStringSize = 18;
    /// @brief Size of a MAC address byte array
    constexpr uint8_t MacAddressByteArraySize = 6;

    /**
     * @brief Convert data from network byte order to host byte order.
     * @param[in] data The data to convert.
     * @returns The data in host byte order.
     */
    template<typename T>
    requires std::is_arithmetic_v<T>
    inline T NetworkToHostByteOrder(const T &data) {
        const uint16_t i = 1;

        //i is 0b00000001 in memory for big endian (network byte order)
        //i is 0b10000000 in memory for little endian
        //So reading as a byte, we'll either get 1 or 0.
        const bool hostIsLittleEndian = (*((char *)&i));
        if (hostIsLittleEndian) {
            T swappedData = 0;
            if (std::is_same<T, uint64_t>::value) {
                swappedData |= (data & 0x00000000000000FFULL) << ToBits(7);
                swappedData |= (data & 0x000000000000FF00ULL) << ToBits(5);
                swappedData |= (data & 0x0000000000FF0000ULL) << ToBits(3);
                swappedData |= (data & 0x00000000FF000000ULL) << ToBits(1);
                swappedData |= (data & 0x000000FF00000000ULL) >> ToBits(1);
                swappedData |= (data & 0x0000FF0000000000ULL) >> ToBits(3);
                swappedData |= (data & 0x00FF000000000000ULL) >> ToBits(5);
                swappedData |= (data & 0xFF00000000000000ULL) >> ToBits(7);
            }
            else if (std::is_same<T, uint32_t>::value) {
                swappedData |= (data & 0x000000FF) << ToBits(3);
                swappedData |= (data & 0x0000FF00) << ToBits(1);
                swappedData |= (data & 0x00FF0000) >> ToBits(1);
                swappedData |= (data & 0xFF000000) >> ToBits(3);
            }
            else if (std::is_same<T, uint16_t>::value) {
                swappedData |= (data & 0x00FF) << ToBits(1);
                swappedData |= (data & 0xFF00) >> ToBits(1);
            }

            return swappedData;
        }

        return data;
    }

    /**
     * @brief Convert data from host byte order to network byte order.
     * @param[in] data The data to convert.
     * @returns The data in network byte order.
     */
    template<typename T>
    inline T HostToNetworkByteOrder(const T &data) {
        return NetworkToHostByteOrder(data);
    }

    /**
     * @struct ConfigurationParameters 
     * @brief Contains the parameters used to configure the network.
     */
    struct ConfigurationParameters {
        public:
        /// @brief The technology type these parameters are meant for
        virtual NetworkTypes::Technology technology() const = 0;
    };
}

#endif // __NETWORK_TYPES_HPP__