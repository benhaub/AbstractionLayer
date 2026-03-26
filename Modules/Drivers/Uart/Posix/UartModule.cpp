#include "UartModule.hpp"
#include "Log.hpp"
//C++
#include <filesystem>

std::array<char, 32> Uart::toDevicePath(const PeripheralNumber peripheralNumber) {
    std::array<char, 32> devicePath = {0};
    std::vector<std::array<char, 32>> discoveredDevicePaths;

    const auto collectMatchingPaths = [&](const char *prefix) {
        const std::filesystem::path directoryPath("/dev");
        std::error_code errorCode;
        const bool doesDirectoryExist = std::filesystem::exists(directoryPath, errorCode);

        if (false == doesDirectoryExist) {
            return;
        }

        const size_t prefixLength = strlen(prefix);
        const auto directoryIterator = std::filesystem::directory_iterator(directoryPath, errorCode);

        if (true == static_cast<bool>(errorCode)) {
            return;
        }

        for (const std::filesystem::directory_entry &entry : directoryIterator) {
            const bool isCharacterDevice = std::filesystem::is_character_file(entry.path(), errorCode);
            const bool didQueryFail = (true == static_cast<bool>(errorCode));

            if (true == didQueryFail) {
                continue;
            }

            if (true == isCharacterDevice) {
                const std::string filename = entry.path().filename().string();
                const bool doesPrefixMatch = (0 == filename.compare(0, prefixLength, prefix));

                if (true == doesPrefixMatch) {
                    std::array<char, 32> discoveredDevicePath = {0};
                    const std::string fullPath = entry.path().string();
                    const size_t fullPathLength = fullPath.size();
                    const bool doesPathFit = (discoveredDevicePath.size() > fullPathLength);

                    if (true == doesPathFit) {
                        strncpy(discoveredDevicePath.data(), fullPath.c_str(), discoveredDevicePath.size() - 1);

                        const bool isPathNonEmpty = (0 < strlen(discoveredDevicePath.data()));

                        if (true == isPathNonEmpty) {
                            discoveredDevicePaths.push_back(discoveredDevicePath);
                        }
                    }
                }
            }
        }
    };

#if defined(__APPLE__)
    collectMatchingPaths("cu.");
    collectMatchingPaths("tty.");
#elif defined(__linux__)
    collectMatchingPaths("ttyUSB");
    collectMatchingPaths("ttyACM");
#endif

    std::sort(discoveredDevicePaths.begin(), discoveredDevicePaths.end(), [](const std::array<char, 32> &leftPath, const std::array<char, 32> &rightPath) {
        return (0 > strcmp(leftPath.data(), rightPath.data()));
    });

    const bool hasDiscoveredUartDevices = (0 < discoveredDevicePaths.size());

    if (true == hasDiscoveredUartDevices) {
        PLT_LOGI(TAG, "Discovered UART devices:");

        for (size_t discoveredPathIndex = 0; discoveredDevicePaths.size() > discoveredPathIndex; discoveredPathIndex++) {
            PLT_LOGI(TAG, "Peripheral number %u -> %s", static_cast<unsigned int>(discoveredPathIndex), discoveredDevicePaths[discoveredPathIndex].data());
        }
    }
    else {
        PLT_LOGI(TAG, "Discovered UART devices: none");
    }

    const size_t peripheralNumberIndex = static_cast<size_t>(static_cast<uint8_t>(peripheralNumber));
    const bool isIndexInRange = (discoveredDevicePaths.size() > peripheralNumberIndex);

    if (true == isIndexInRange) {
        devicePath = discoveredDevicePaths[peripheralNumberIndex];
    }
    else {
        PLT_LOGE(TAG, "Requested peripheral number is out of range <requested:%u, discovered:%u>",
            static_cast<unsigned int>(peripheralNumberIndex),
            static_cast<unsigned int>(discoveredDevicePaths.size()));
    }

    return devicePath;
}

ErrorType Uart::setTermiosConfig(struct termios &tty) {
    tty = {};
    ErrorType error = ErrorType::Success;

    error = toPosixBaudRate(uartParams().driverConfig.baudRate, tty);

    if (ErrorType::Success == error) {
        error = toPosixDataBits(uartParams().driverConfig.dataBits, tty);

        if (ErrorType::Success == error) {
            error = toPosixParity(uartParams().driverConfig.parity, tty);

            if (ErrorType::Success == error) {
                error = toPosixStopBits(uartParams().driverConfig.stopBits, tty);

                if (ErrorType::Success == error) {
                    error = toPosixFlowControl(uartParams().driverConfig.flowControl, tty);

                    if (ErrorType::Success == error) {
                        tty.c_cc[VMIN]  = 0;
                        tty.c_cc[VTIME] = 5;
                        tty.c_cflag |= (CLOCAL | CREAD);

                        if (0 != tcsetattr(_fileDescriptor, TCSANOW, &tty)) {
                            close(_fileDescriptor);
                            _fileDescriptor = -1;
                            error = ErrorType::Failure;
                        }
                    }
                }
            }
        }
    }

    return error;
}

ErrorType Uart::init() {
    ErrorType error = ErrorType::Failure;

    _devicePath = toDevicePath(uartParams().hardwareConfig.peripheralNumber);

    if (strlen(_devicePath.data()) > 0) {
        _fileDescriptor = open(_devicePath.data(), O_RDWR | O_NONBLOCK);

        if (-1 != _fileDescriptor) {
            struct termios tty;
            error = setTermiosConfig(tty);
        }
        else {
            error = fromPlatformError(errno);
        }
    }

    return error;
}

ErrorType Uart::deinit() {
    close(_fileDescriptor);

    return ErrorType::Success;
}

ErrorType Uart::txBlocking(const StaticString::Container &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return txBlocking(data->c_str(), data->size(), timeout);
}
ErrorType Uart::txBlocking(std::string_view data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return txBlocking(data.data(), data.size(), timeout);
}
ErrorType Uart::txBlocking(const char *data, const size_t size, const Milliseconds timeout) {
    if (_fileDescriptor < 0) {
        return ErrorType::PrerequisitesNotMet;
    }
    
    ssize_t bytesWritten = write(_fileDescriptor, data, size);

    if (bytesWritten < 0) {
        return ErrorType::Failure;
    }
    else if (bytesWritten > 0 && static_cast<size_t>(bytesWritten) < size) {
        return ErrorType::LimitReached;
    }
    else if (bytesWritten > 0 && static_cast<size_t>(bytesWritten) == size) {
        return ErrorType::Success;
    }
    else {
        return ErrorType::Failure;
    }
    
}

ErrorType Uart::rxBlocking(StaticString::Container &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    size_t bytesRead = 0;
    ErrorType error = ErrorType::Failure;

    if (ErrorType::Success == (error = rxBlocking(&buffer[0], buffer->size(), bytesRead, timeout))) {
        buffer->resize(bytesRead);
    }

    return error;
}
ErrorType Uart::rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    size_t bytesRead = 0;
    ErrorType error = ErrorType::Failure;

    if (ErrorType::Success == (error = rxBlocking(&buffer[0], buffer.size(), bytesRead, timeout))) {
        buffer.resize(bytesRead);
    }

    return error;
}
ErrorType Uart::rxBlocking(char *buffer, const size_t bufferSize, size_t &bytesRead, const Milliseconds timeout) {
    if (_fileDescriptor < 0) {
        return ErrorType::PrerequisitesNotMet;
    }
    
    fd_set readfds;
    struct timeval tv;
    bytesRead = 0;
    
    FD_ZERO(&readfds);
    FD_SET(_fileDescriptor, &readfds);
    
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    
    int selectResult = select(_fileDescriptor + 1, &readfds, NULL, NULL, &tv);
    if (selectResult < 0) {
        return ErrorType::Failure;
    }
    else if (selectResult == 0) {
        return ErrorType::Timeout;
    }
    
    bytesRead = read(_fileDescriptor, buffer, bufferSize);
    if (static_cast<ssize_t>(bytesRead) > 0) {
        return ErrorType::Success;
    }
    else {
        return ErrorType::NoData;
    }
}

ErrorType Uart::txNonBlocking(const std::shared_ptr<StaticString::Container> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotAvailable;
}
ErrorType Uart::txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotAvailable;
}

ErrorType Uart::rxNonBlocking(std::shared_ptr<StaticString::Container> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<StaticString::Container> buffer)> callback) {
    return ErrorType::NotAvailable;
}
ErrorType Uart::rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotAvailable;
}

ErrorType Uart::flushRxBuffer() {
    if (_fileDescriptor < 0) {
        return ErrorType::PrerequisitesNotMet;
    }
    
    // Flush the input buffer
    if (tcflush(_fileDescriptor, TCIFLUSH) != 0) {
        return ErrorType::Failure;
    }
    
    return ErrorType::Success;
}