#include "NtpClient.hpp"
//AbstractionLayer
#include "NetworkAbstraction.hpp"

ErrorType NtpClient::connectToServer(const NtpClientTypes::NtpServerHostName hostname, const Port port) {
    Socket socket = -1;
    Milliseconds timeout = 30*1000;

    ErrorType error = ErrorType::Success;
    ErrorType connectError = ErrorType::Success;
    connectError = ipClient().connectTo(NtpClientTypes::ntpServerHostNameToCharArray(hostname, error).data(), port, IpTypes::Protocol::Udp, IpTypes::Version::IPv4, socket, timeout);

    if (ErrorType::Success != error) {
        return error;
    }
    else {
        return connectError;
    }
}

ErrorType NtpClient::disconnect() {
    return ipClient().disconnect();
}

/*
 * Adapted from: https://github.com/lettier/ntpclient/tree/master
 */
UnixTime NtpClient::currentTime(ErrorType &error, const Milliseconds timeout, const NtpClientTypes::LeapIndicator ntpLeapIndicator, const uint8_t ntpVersion, const NtpClientTypes::AssociationMode ntpMode) {
    //48-byte NTP packet
    struct NtpPacket {
        uint8_t mode : 3                    = 0; //Mode
        uint8_t version : 3                 = 0; //Version number
        uint8_t leapIndicator : 2           = 0; //Leap indicator
        uint8_t stratum                     = 0; //Stratum
        uint8_t maxIntervalBetweenMessages  = 0; //Poll
        uint8_t localClockPrecision         = 0; //Precision
        uint32_t totalRoundTripDelayTime    = 0; //Root delay
        uint32_t maxPrimaryClockSourceError = 0; //Root dispersion
        uint32_t referenceClockIdentifier   = 0; //Reference clock identifier
        uint32_t referenceTimestampSeconds  = 0; //Reference timestamp
        uint32_t referenceTimestampFraction = 0; //Reference timestamp
        uint32_t originateTimestampSeconds  = 0; //Origin timestamp
        uint32_t originateTimestampFraction = 0; //Origin timestamp
        uint32_t receiveTimestampSeconds    = 0; //Receive timestamp
        uint32_t receiveTimestampFraction   = 0; //Receive timestamp
        uint32_t transmitTimestampSeconds   = 0; //Transmit timestamp
        uint32_t transmitTimestampFraction  = 0; //Transmit timestamp
    };

    std::string packet(sizeof(NtpPacket), 0);
    packet[0] = static_cast<uint8_t>(ntpLeapIndicator) << 6 | static_cast<uint8_t>(ntpVersion) << 3 | static_cast<uint8_t>(ntpMode);

    UnixTime unixTimeUtc = 0;
    if (ErrorType::Success == (error = ipClient().sendBlocking(packet, timeout))) {
        std::string &buffer = packet;
        if (ErrorType::Success == (error = ipClient().receiveBlocking(buffer, timeout))) {
            const NtpPacket &ntpPacket = *reinterpret_cast<NtpPacket *>(buffer.data());
            constexpr uint32_t seventyYearsInSeconds = 2208988800;

            uint32_t receiveTimestampSeconds = NetworkTypes::NetworkToHostByteOrder(ntpPacket.receiveTimestampSeconds);
            //NTP epoch starts from 1900, so take off the seventy year difference to get unix epoch
            unixTimeUtc = receiveTimestampSeconds - seventyYearsInSeconds;
            error = ErrorType::Success;
        }
    }

    return unixTimeUtc;
}