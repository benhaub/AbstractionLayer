//AbstractionLayer Modules
#include "IpCellularClientModule.hpp"
#include "CellularModule.hpp"
#include "OperatingSystemModule.hpp"
//AbstractionLayer Applications
#include "Log.hpp"
//C++
#include <cstring>
#include <cmath>

#define IP_CELLULAR_CLIENT_DEBUG 0

/**
 * @brief Convert the IP client version to the Quectel context type
 * 
 * @param version The IP client version
 * @return The Quectel context type
 */
static ContextType ToQuectelContextType(const IpClientTypes::Version version) {
    switch (version) {
        case IpClientTypes::Version::IPv4:
            return ContextType::Ipv4;
        case IpClientTypes::Version::IPv6:
            return ContextType::Ipv6;
        default:
            return ContextType::Unknown;
    }
}

static std::string ToQuectelProtocol(const IpClientTypes::Protocol protocol) {
    switch (protocol) {
        case IpClientTypes::Protocol::Tcp:
            return "TCP";
        case IpClientTypes::Protocol::Udp:
            return "UDP";
        default:
            return "";
    }
}

static int8_t ToQuectelAccessMode(const CellularConfig::AccessMode accessMode) {
    switch (accessMode) {
        case CellularConfig::AccessMode::Buffer:
            return 0;
        case CellularConfig::AccessMode::DirectPush:
            return 1;
        case CellularConfig::AccessMode::Transparent:
            return 2;
        default:
            return -1;
    }
}

ErrorType IpCellularClient::connectTo(std::string_view hostname, const Port port, const IpClientTypes::Protocol protocol, const IpClientTypes::Version version, Socket &socket, const Milliseconds timeout) {
    bool doneConnecting = false;
    ErrorType error = ErrorType::Failure;

    auto connectCb = [&]() -> ErrorType {
        _cellNetworkInterface = dynamic_cast<Cellular *>(&network());
        if (nullptr == _cellNetworkInterface) {
            PLT_LOGE(TAG, "Can't connect to network without a cellular interface");
            return ErrorType::NotSupported;
        }

        disconnect();

        if (version != IpClientTypes::Version::IPv4) {
            PLT_LOGE(TAG, "only IPv4 is supported");
            error = ErrorType::NotSupported;
            doneConnecting = true;
            return error;
        }

        if (ErrorType::LimitReached == _cellNetworkInterface->nextAvailableConnectionId(socket)) {
            PLT_LOGW(TAG, "No more sockets available");
            socket = -1;
            error = ErrorType::LimitReached;
            doneConnecting = true;
            return error;
        }

        ErrorType error = _cellNetworkInterface->pdpContextIsActive(Cellular::_IpContext);
        const bool pdpContextIsNotActive = ErrorType::Success != error;
        if (pdpContextIsNotActive) {
            error = _cellNetworkInterface->activatePdpContext(Cellular::_IpContext, socket, ToQuectelContextType(version), _cellNetworkInterface->accessPointName());
            const bool pdpContextCouldNotBeActivated = ErrorType::Success != error;
            if (pdpContextCouldNotBeActivated) {
                socket = -1;
                PLT_LOGW(TAG, "Failed to activate PDP context");
                doneConnecting = true;
                return error;
            }
        }

        std::string quectelProtocol = ToQuectelProtocol(protocol);
        if (quectelProtocol.empty()) {
            PLT_LOGE(TAG, "Invalid protocol");
            socket = -1;
            error = ErrorType::InvalidParameter;
            doneConnecting = true;
            return error;
        }

        std::string openSocketCommand("AT+QIOPEN=");
        openSocketCommand.append(std::to_string(Cellular::_IpContext));
        openSocketCommand.append(",").append(std::to_string(socket));
        openSocketCommand.append(",").append("\"").append(quectelProtocol).append("\"");
        openSocketCommand.append(",").append("\"").append(hostname.data()).append("\"");
        openSocketCommand.append(",").append(std::to_string(port));
        openSocketCommand.append(",").append(std::to_string(0));
        openSocketCommand.append(",").append(std::to_string(ToQuectelAccessMode(_cellNetworkInterface->accessModeConst())));

        error = _cellNetworkInterface->sendCommand(openSocketCommand, 1000, 10);
        if (ErrorType::Success != error) {
            socket = -1;
            PLT_LOGW(TAG, "AT command failed <error:%u>", openSocketCommand);
            doneConnecting = true;
            return error;
        }

        std::string &receiveBuffer = openSocketCommand;
        receiveBuffer.resize(receiveBuffer.capacity());

        error = _cellNetworkInterface->receiveCommand(openSocketCommand, 1000, 10, "OK");
        if (ErrorType::Success != error) {
            socket = -1;
            PLT_LOGW(TAG, "AT command failed <error:%u>", openSocketCommand);
            PLT_LOG_BUFFER_HEXDUMP(TAG, receiveBuffer.data(), receiveBuffer.size(), LogType::Warning);
            doneConnecting = true;
            return error;
        }

        _status.connected = true;
        _socket = socket;

        return ErrorType::Success;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(connectCb));
    if (ErrorType::Success != network().addEvent(event)) {
        return ErrorType::Failure;
    }

    while (!doneConnecting) {
        OperatingSystem::Instance().delay(10);
    }

    return error;
}

ErrorType IpCellularClient::disconnect() {
    _status.connected = false;
    return ErrorType::Success;
}

//TODO: Not thread safe. Should be using the netowrk thread to perform this.
ErrorType IpCellularClient::sendBlocking(const std::string &data, const Milliseconds timeout) {

    switch (_cellNetworkInterface->accessModeConst()) {
        case CellularConfig::AccessMode::Transparent: {
            return _cellNetworkInterface->_ic->txBlocking(data, timeout);
        }
        case CellularConfig::AccessMode::Buffer:
        case CellularConfig::AccessMode::DirectPush: {
            constexpr Milliseconds timeout = 1000;
            constexpr Count maxRetries = 10;

            ErrorType error = ErrorType::Failure;
            std::string receiveBuffer(128, 0);
            Bytes remaining = data.size();

            std::string command("AT+QISEND=");
            command.append(std::to_string(_socket));
            command.append(",").append(std::to_string(data.size()));

            error = _cellNetworkInterface->sendCommand(command, timeout, 10);
            if (ErrorType::Success != error) {
                PLT_LOGW(TAG, "AT command error:");
                PLT_LOG_BUFFER_HEXDUMP(TAG, command.c_str(), command.size(), LogType::Warning);
                return error;
            }

            error = _cellNetworkInterface->receiveCommand(receiveBuffer, 1000, 10, ">");
            if (ErrorType::Timeout == error) {
                PLT_LOGW(TAG, "Failed to receive command:");
                PLT_LOG_BUFFER_HEXDUMP(TAG, command.c_str(), command.size(), LogType::Warning);
            }

            const uint16_t maxSegmentSize = 1460;
            if (data.size() < maxSegmentSize) {
                error = _cellNetworkInterface->_ic->txBlocking(data, timeout);
                if (ErrorType::Success != error) {
                    PLT_LOGW(TAG, "Failed to send data");
                    return error;
                }

                error = _cellNetworkInterface->receiveCommand(receiveBuffer, timeout, maxRetries, "SEND OK");
                if (ErrorType::Timeout == error) {
                    PLT_LOGW(TAG, "Receive error: SEND OK");
                    PLT_LOG_BUFFER_HEXDUMP(TAG, receiveBuffer.c_str(), receiveBuffer.size(), LogType::Warning);
                    return error;
                }
            }
            else {
                Count sendRetries = 0;
                do {
                    _cellNetworkInterface->_ic->txBlocking(data.data() + (data.size() - remaining), remaining > maxSegmentSize ? remaining : maxSegmentSize);
                    error = _cellNetworkInterface->receiveCommand(receiveBuffer, timeout, maxRetries, "SEND OK");
                    if (ErrorType::Timeout == error) {
                        PLT_LOGW(TAG, "Receive error: SEND OK");
                        PLT_LOG_BUFFER_HEXDUMP(TAG, receiveBuffer.c_str(), receiveBuffer.size(), LogType::Warning);
                        return error;
                    }
                } while (remaining > 0 && sendRetries++ < maxRetries);
            }

            return ErrorType::Success;
        }
    }

    return ErrorType::NotSupported;
}

//TODO: Not thread safe. Should be using the netowrk thread to perform this.
ErrorType IpCellularClient::receiveBlocking(std::string &buffer, const Milliseconds timeout) {
    constexpr Count maxRetries = 10;

    switch (_cellNetworkInterface->accessModeConst()) {
        case CellularConfig::AccessMode::Transparent: {
            return _cellNetworkInterface->receiveCommand(buffer, timeout, maxRetries);
        }
        case CellularConfig::AccessMode::DirectPush: {
            ErrorType error = ErrorType::Failure;
            error = _cellNetworkInterface->receiveCommand(buffer, timeout, maxRetries, "+QIURC:");
            if (ErrorType::Timeout == error) {
                return error;
            }

            return ErrorType::NotImplemented;
        }
        case CellularConfig::AccessMode::Buffer: {
            ErrorType error = pollForData(_socket, timeout);
            if (ErrorType::Success != error) {
                return error;
            }

            constexpr Count maxRetries = 10;
            constexpr Milliseconds timeout = 1000;

            std::string readCommand("AT+QIRD=");
            readCommand.append(std::to_string(_socket));
            readCommand.append(",");
            const bool needsMultipleReads = buffer.size() > _cellNetworkInterface->_MaxBytesToRead;
            readCommand.append(std::to_string(needsMultipleReads ? _cellNetworkInterface->_MaxBytesToRead : buffer.size()));

            error = _cellNetworkInterface->sendCommand(readCommand, maxRetries, timeout);
            if (ErrorType::Success != error) {
                PLT_LOGW(TAG, "AT command error:");
                PLT_LOG_BUFFER_HEXDUMP(TAG, readCommand.c_str(), readCommand.size(), LogType::Warning);
                return error;
            }

            if (needsMultipleReads) {
                const Bytes totalToRead = buffer.size();
                buffer.resize(0);
                const Count numberOfReadsNeeded = std::ceil(totalToRead / _cellNetworkInterface->_MaxBytesToRead);

                for (Count i = 0; i < numberOfReadsNeeded; i++) {
                    std::string multipleReadBuffer(_cellNetworkInterface->_MaxBytesToRead, 0);

                    error = _cellNetworkInterface->receiveCommand(multipleReadBuffer, maxRetries, timeout, "OK");

                    if (ErrorType::Timeout == error) {
                        PLT_LOGW(TAG, "AT command error:");
                        PLT_LOG_BUFFER_HEXDUMP(TAG, readCommand.c_str(), readCommand.size(), LogType::Warning);
                        return error;
                    }

                    buffer.append(multipleReadBuffer);
                }
            }
            else {
                error = _cellNetworkInterface->receiveCommand(buffer, maxRetries, timeout, "OK");

                if (ErrorType::Timeout == error) {
                    PLT_LOGW(TAG, "AT command error:");
                    PLT_LOG_BUFFER_HEXDUMP(TAG, readCommand.c_str(), readCommand.size(), LogType::Warning);
                    return error;
                }
            }

            //Pg. 20, Sec. 2.1.8, Quectel LTE TCP/IP standard.
            //Incoming payload response begins with a preamble of "+QIRD: <bytesReceived>". So from the beginning of the buffer remove everything until the first " ".
            for (size_t i = 0; i < buffer.size(); i++) {
                if (buffer.at(i) == ' ') {
                    i++;
                    memmove(&buffer.at(0), &buffer.at(i), buffer.size() - i);
                    buffer.resize(buffer.size() - i);
                    break;
                }
            }
            //This leaves us with the buffer beginning with "<bytesReceived>". Replace the command line termination character with a null terminator
            //so that we can use the std::strtoul function to parse the number of bytes received.
            for (size_t i = 0; i < buffer.size(); i++) {
                if (buffer.at(i) == _cellNetworkInterface->_commandLineTerminationCharacter) {
                    buffer.at(i) = '\0';

                    const Bytes toRead = std::strtoul(buffer.c_str(), nullptr, 10);
                    if (toRead > buffer.size()) {
                        PLT_LOGW(TAG, "Bytes to read is larger than the buffer size <toRead:%u, bufferSize:%u>", toRead, buffer.size());
                        return ErrorType::PrerequisitesNotMet;
                    }
                    if (0 == toRead) {
                        buffer.resize(0);
                        return ErrorType::NoData;
                    }
#if IP_CELLULAR_CLIENT_DEBUG
                    PLT_LOGI(TAG, "Bytes to read: %u", toRead);
#endif
                    //Move the payload over and overwrite the remaining preamble so that all we've got left is the payload.
                    //There is still the OK message at the end, but we can just reduce the buffer size to remove it.
                    i += 2;
                    memmove(&buffer.at(0), &buffer.at(i), buffer.size() - i);
                    buffer.resize(toRead);
                    return ErrorType::Success;
                }
            }

            return ErrorType::Failure;
        }
        default:
            return ErrorType::NotSupported;
    }
}

ErrorType IpCellularClient::sendNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    auto tx = [this, callback](const std::shared_ptr<std::string> frame, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        if (nullptr == frame.get()) {
            assert(false);
            return ErrorType::NoData;
        }

        error = sendBlocking(*frame, timeout);

        if (nullptr != callback) {
            callback(error, frame->size());
        }

        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(tx, data, timeout));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    return ErrorType::Success;
}

ErrorType IpCellularClient::receiveNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    auto rx = [this, callback](const std::shared_ptr<std::string> buffer, const Milliseconds timeout) -> ErrorType {
        ErrorType error = ErrorType::Failure;

        if (nullptr == buffer.get()) {
            assert(false);
            return ErrorType::NoData;
        }

        error = receiveBlocking(*buffer, timeout);

        if (nullptr != callback) {
            callback(error, buffer);
        }

        return error;
    };

    EventQueue::Event event = EventQueue::Event(std::bind(rx, buffer, timeout));
    ErrorType error = network().addEvent(event);
    if (ErrorType::Success != error) {
        return error;
    }

    return ErrorType::Success;
}

ErrorType IpCellularClient::pollForData(const Socket socket, const Milliseconds timeout) {
    if (CellularConfig::AccessMode::Buffer != _cellNetworkInterface->accessModeConst()) {
        return ErrorType::PrerequisitesNotMet;
    }

    Ticks timeoutTicks;
    Milliseconds waitForIncomingMessage;
    Milliseconds timeNow;

    OperatingSystem::Instance().getSystemTick(timeoutTicks);
    OperatingSystem::Instance().ticksToMilliseconds(timeoutTicks, waitForIncomingMessage);
    timeNow = waitForIncomingMessage;
    waitForIncomingMessage += timeout;

    while(timeNow <= timeout) {
        if (ErrorType::Success == _cellNetworkInterface->dataIsAvailable(socket)) {
            break;
        }

        Ticks tick;
        OperatingSystem::Instance().getSystemTick(tick);
        OperatingSystem::Instance().ticksToMilliseconds(tick, timeNow);
    }

    if (timeNow > timeout) {
        PLT_LOGW(TAG, "No data available to read.");
        return ErrorType::Timeout;
    }

    return ErrorType::Success;
}
