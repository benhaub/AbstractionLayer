/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   NtpClient.hpp
* @details An NTP client.
* @ingroup Applications
*******************************************************************************/
#ifndef __NTP_CLIENT_HPP__
#define __NTP_CLIENT_HPP__

//AbstractionLayer
#include "IpClient.hpp"

/**
 * @namespace NtpClientTypes
 * @brief Types for the NTP client.
 */
namespace NtpClientTypes {

    /**
     * @enum NtpServerHostName
     * @brief NTP servers that can be connected to.
     */
    enum class NtpServerHostName {
        Unknown = 0, ///< Unknown server
        PoolNtpOrg,  ///< Pool NTP org
        GoogleNtp,   ///< Google NTP
        MicrosoftNtp,///< Microsoft NTP
        AppleNtp,    ///< Apple NTP
        NistNtp      ///< National Institute of Standards and Technology NTP
    };

    /**
     * @enum LeapIndicator
     * @brief The leap indicator for the NTP packet.
     */
    enum class LeapIndicator {
        NoWarning = 0,
        LastMinute61,
        LastMinute59,
        Unknown
    };

    /**
     * @enum AssociationMode
     * @brief The mode for the NTP packet.
     */
    enum class AssociationMode {
        Reserved = 0,
        SymmetricActive,
        SymmetricPassive,
        Client,
        Server,
        Broadcast,
        Control,
        Private,
        Unknown
    };
        

    /// @brief NtpVersion
    constexpr uint8_t NtpVersion = 3;
    /// @brief The max size of the NTP server host name.
    constexpr Count NtpServerHostNameSize = 17;

    /**
     * @brief Convert a NTP server host name to a char array.
     * @param hostname The NTP server host name to convert.
     * @param error The error type to return. May one of:\n
     *              - ErrorType::Success\n
     *              - ErrorType::NotImplemented\n
     * @returns A char array containing the NTP server host name.
     * @returns An empty char array if the NTP server host name is not supported.
     * @sa NtpClientTypes::NtpServerHostName
     */
    constexpr inline std::array<char, NtpServerHostNameSize> ntpServerHostNameToCharArray(const NtpServerHostName hostname, ErrorType &error) {
        error = ErrorType::Success;

        switch (hostname) {
            case NtpServerHostName::PoolNtpOrg:
                return std::array<char, NtpServerHostNameSize>{"pool.ntp.org"};
            case NtpServerHostName::GoogleNtp:
                return std::array<char, NtpServerHostNameSize>{"time.google.com"};
            case NtpServerHostName::MicrosoftNtp:
                return std::array<char, NtpServerHostNameSize>{"time.windows.com"};
            case NtpServerHostName::AppleNtp:
                return std::array<char, NtpServerHostNameSize>{"time.apple.com"};
            case NtpServerHostName::NistNtp:
                return std::array<char, NtpServerHostNameSize>{"time.nist.gov"};
            default:
                error = ErrorType::NotImplemented;
                return std::array<char, NtpServerHostNameSize>{};
        }
    }
}

/**
 * @class NtpClient
 * @brief An Network Time Protocol client.
 */
class NtpClient {

    public:
    /// @brief Default constructor.
    NtpClient() = default;
    /// @brief Disconnect from the NTP server.
    ~NtpClient() {
        disconnect();
    }

    /**
     * @brief Connect to a NTP server.
     * @param hostname The NTP server to connect to.
     * @param port The port to connect to. Default is 123.
     * @returns Any errors returned by IpClient::connectTo.
     * @returns Any errors returned by NtpClientTypes::ntpServerHostNameToCharArray.
     * @sa IpClient::connectTo
     * @sa NtpClientTypes::ntpServerHostNameToCharArray
     */
    ErrorType connectToServer(const NtpClientTypes::NtpServerHostName hostname, const Port port = 123);
    /**
     * @brief Get the current time from the NTP server.
     * @param error The error that occurred while getting the time
     * @param timeout The timeout for the NTP request.
     * @param ntpLeapIndicator The leap indicator for the NTP packet. Default is No warning.
     * @param ntpVersion The NTP version for the NTP packet. Default is 3.
     * @param ntpMode The mode for the NTP packet. Default is Symmetric Passive.
     * @returns The current time from the NTP server. Valid only if the error is ErrorType::Success.
     */
    UnixTime currentTime(ErrorType &error, const Milliseconds timeout,
                        const NtpClientTypes::LeapIndicator ntpLeapIndicator = NtpClientTypes::LeapIndicator::NoWarning,
                        const uint8_t ntpVersion = NtpClientTypes::NtpVersion,
                        const NtpClientTypes::AssociationMode ntpMode = NtpClientTypes::AssociationMode::Client);

    /// @brief Get a mutable reference to the NTP client.
    IpClient &ipClient() { return _ipClient; }

    private:
    /// @brief The IP client.
    IpClient _ipClient = IpClient();

    /**
     * @brief Disconnect from the NTP server.
     * @returns Any errors returned by IpClient::disconnect.
     * @sa IpClient::disconnect
     */
    ErrorType disconnect();
};

#endif // __NTP_CLIENT_HPP__