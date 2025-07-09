/***************************************************************************//**
* @author   Ben Haubrich
* @file     CellularModule.hpp
* @details  Cellular for Quectel EC21A
* @ingroup  NetworkModules
*******************************************************************************/
#ifndef __CELLULAR_MODULE_HPP__
#define __CELLULAR_MODULE_HPP__

#include "CellularAbstraction.hpp"
#include "GpioModule.hpp"

class IpCellularClient;

using PdpContext = int8_t;

enum class ContextType : uint8_t {
    Unknown = 0,
    Ipv4,
    Ipv6,
    Ipv4v6
};

class Cellular final : public CellularAbstraction {
    public:
    Cellular() : CellularAbstraction() {
        _connectionIds.fill(-1);
    }

    ErrorType init() override;
    ErrorType networkUp() override;
    ErrorType networkDown() override;
    
    ErrorType txBlocking(const std::string &frame, const Socket socket, const Milliseconds timeout) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> frame, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType rxBlocking(std::string &frameBuffer, const Socket socket, const Milliseconds timeout) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> frameBuffer, const Socket socket, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> frameBuffer)> callback) override;
    ErrorType getMacAddress(std::array<char, NetworkTypes::MacAddressStringSize> &macAddress) override;
    ErrorType getSignalStrength(DecibelMilliWatts &signalStrength) override;

    ErrorType mainLoop() override;

    ErrorType reset() override;

    /**
     * @brief Allow the IpCellularClient to access the private members of Cellular
     * @details In order for the client to connect, it will need to be able to send AT commands to the modem.
    */
    friend IpCellularClient;

    private:
    /**
     * @brief The context for the IP socket.
     * @details Pg. 11 Quectel LTE Standard TCP/IP Application Note. There are a maximum of 3 contexts available.
     *          Each context can open 11 connection IDs (or sockets)
     */
    static constexpr PdpContext _IpContext = 1;
    /**
     * @brief The context for the HTTP socket.
     * @sa ipContext
     */
    static constexpr PdpContext _HttpContext = 2;
    /// @brief The maximum number of contexts.
    static constexpr PdpContext _MaxContexts = 3;
    /// @brief The maximum number of sockets per context.
    static constexpr Socket _MaxSocketsPerContext = 11;
    /// @brief Pg.20, Sect. 2.1.8 Quectel LTE TCP/IP Standard. Maximum number of bytes that can be read at a time.
    static constexpr Bytes _MaxBytesToRead = 1500;
    /// @brief The GPIO pin for the reset pin.
    std::unique_ptr<Gpio> _gpioReset;
    /**
     * @brief The line termination character. Set to the default on Pg. 23 of the EC21A AT Command Manual.
     * @details This is the character used to terminate an AT command.
     */
    char _commandLineTerminationCharacter = '\r';
    /**
     * @brief The response formatting character. Set to the default on Pg. 24 of the EC21A AT Command Manual.
     * @details This is the character used along with the _commandLineTerminationCharacter to terminate the response from the modem.
     *          Responses are terminated with the _commandLineTerminationCharacter followed by the _responseFormattingCharacter.
     */
    char _responseFormattingCharacter = '\n';
    /// @brief The connection ids.
    std::array<Socket, 11> _connectionIds;

    /**
     * @brief Send an AT command to the modem and wait for a response.
     * @param[in] atCommand The AT command to send.
     * @param[in] timeout The timeout for the command.
     * @param[in] maxRetries The maximum number of retries to send the command.
     * @param[out] responseBuffer The buffer to store the response in.
     * @returns ErrorType::Success if the command was sent and a response was received.
     * @returns ErrorType::Failure if the command was not sent or no response was received.
    */
    ErrorType sendCommand(const std::string &atCommand, const Milliseconds timeout, const Count maxRetries);

    /**
     * @brief Receive a response from the modem.
     * @details If ErrorType::Failure is returned, then you may want to try calling this function again until the response is received.
     * @param[out] responseBuffer The buffer to store the response in.
     * @param[in] timeout The timeout for the response.
     * @param[in] maxRetries The maximum number of retries to receive the response.
     * @param[in] expectedResponse Optional. The expected response from the modem. If not provided, this function will ensure that the response ends with the response formatting character.
     *            but will not check for response messages such as "OK" or "ERROR".
     * @returns ErrorType::Success if the response was received.
     * @returns ErrorType::Timeout if receiving from the IC failed more times than maxRetries.
     * @returns ErrorType::Failure if the expected response was not found.
    */
    ErrorType receiveCommand(std::string &responseBuffer, const Milliseconds timeout, const Count maxRetries, const std::string expectedResponse = std::string());

    /**
     * @brief Get the manufacturer name of the modem.
     * @param[out] mfgName The string to store the manufacturer name in.
     * @returns ErrorType::Success if the manufacturer name was retrieved successfully.
     * @returns ErrorType::Failure if the manufacturer name was not retrieved successfully.
    */
    ErrorType getManufacturerName(std::string &mfgName);

    /**
     * @brief Check if the SIM card is inserted.
     * @returns ErrorType::Success if the SIM card is inserted.
     * @returns ErrorType::Failure if the SIM card is not inserted. 
    */
    ErrorType simCardIsInserted();

    /**
     * @brief Check if the pdp context is active.
     * @details A pdp context is like a network interface within the modem. There are 3 of them and each can support up to 11 simultaneous connections.
     * @returns ErrorType::Success if the pdp context is active.
     * @returns ErrorType::Failure if the pdp context is not active.
     * @returns ErrorType's listed in sendCommand
     * @returns ErrorType's listed in receiveCommand
    */
    ErrorType pdpContextIsActive(const PdpContext context);

    /**
     * @brief Check if there is data available to read from the provided socket.
     * @param[in] socket The socket to check for available data.
     * @returns ErrorType::Success if there is data available.
     * @returns ErrorType::NoData if there is no data.
     * @returns Any error returned by sendCommand or receiveCommand if there was an AT command failure.
     */
    ErrorType dataIsAvailable(const Socket socket);

    /**
     * @brief Activate a pdp context.
     * @param[in] context The pdp context to activate.
     * @returns ErrorType::Success if the pdp context was activated successfully.
     * @returns ErrorType::Failure if the pdp context was not activated successfully.
     */
    ErrorType activatePdpContext(const PdpContext context, const Socket socket, const ContextType contextType, const std::string &accessPointName);

    /**
     * @brief Deactivate a pdp context.
     * @param[in] context The pdp context to deactivate.
     * @returns ErrorType::Success if the pdp context was deactivated successfully.
     * @returns ErrorType::Failure if the pdp context was not deactivated successfully.
    */
    ErrorType deactivatePdpContext(const PdpContext context);
    
    /**
     * @brief Set the echo mode of the modem.
     * @details Controls whether the modem will echo back the characters it receives. See Pg. 22 of the EC21A AT Command Manual.
     * @param[in] enable True to enable echo mode, false to disable.
     * @returns ErrorType::Success if the echo mode was set successfully.
     * @returns ErrorType::Failure if the echo mode was not set successfully.
    */
    ErrorType echoMode(const bool enable);

    /**
     * @brief terminatingCharacter
     * @details Pg. 23 EC21A AT Command Manual
     * @param[out] terminatingCharacter The character that the modem will use to terminate the response.
     * @returns ErrorType::Success if the terminating character was retrieved successfully.
     * @returns ErrorType::Failure if the terminating character was not retrieved successfully.
    */
    ErrorType terminatingCharacter(char &terminatingCharacter);

    /**
     * @brief responseFormattingCharacter
     * @details Pg. 24 EC21A AT Command Manual
     * @param[out] responseFormattingCharacter The character that the modem will use to format the response.
     * @returns ErrorType::Success if the response formatting character was retrieved successfully.
     * @returns ErrorType::Failure if the response formatting character was not retrieved successfully.
    */
    ErrorType responseFormattingCharacter(char &responseFormattingCharacter);

    /**
     * @brief Find the next available connection id (aka socket)
     * @param id [out] The next available connection id
     * @return ErrorType::Success if an id was found
     * @return ErrorType::LimitReached if no ids are available
     */
    ErrorType nextAvailableConnectionId(Socket &id) {

        for (unsigned int i = 0; i < _connectionIds.size(); i++) {
            if (-1 == _connectionIds[i]) {
                id = i;
                return ErrorType::Success;
            }
        }

        return ErrorType::LimitReached;
    }

    /**
     * @brief Release a connection id (aka socket)
     * @param id The connection id to release
     */
    void releaseConnectionId(const Socket id) {
        _connectionIds[id] = -1;
    }
};

#endif // __CELLULAR_MODULE_HPP__