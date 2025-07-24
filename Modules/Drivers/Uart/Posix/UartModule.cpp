#include "UartModule.hpp"
//C++
#include <cstdio>
#include <cerrno>

ErrorType Uart::init() {
    ErrorType error = ErrorType::Failure;

    error = discoverSerialDevices(uartParams().hardwareConfig.peripheralNumber);
    if (ErrorType::Success == error) {
        _fileDescriptor = open(_devicePath.c_str(), O_RDWR | O_NONBLOCK);

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

ErrorType Uart::txBlocking(const std::string &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    if (_fileDescriptor < 0) {
        return ErrorType::PrerequisitesNotMet;
    }
    
    ssize_t bytesWritten = write(_fileDescriptor, data.data(), data.size());
    if (bytesWritten < 0) {
        return ErrorType::Failure;
    }
    
    return ErrorType::Success;
}

ErrorType Uart::rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    if (_fileDescriptor < 0) {
        return ErrorType::PrerequisitesNotMet;
    }
    
    fd_set readfds;
    struct timeval tv;
    
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
    
    ssize_t bytesRead = read(_fileDescriptor, &buffer[0], buffer.size());
    if (bytesRead > 0) {
        buffer.resize(bytesRead);
        return ErrorType::Success;
    }
    else {
        return ErrorType::NoData;
    }
}

ErrorType Uart::txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    // For now, implement as blocking but in a separate thread
    // In a real implementation, you'd want to use async I/O
    return ErrorType::NotAvailable;
}

ErrorType Uart::rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    // For now, implement as blocking but in a separate thread
    // In a real implementation, you'd want to use async I/O
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

ErrorType Uart::discoverSerialDevices(const PeripheralNumber peripheralNumber) {
    _devicePath = toDevicePath(peripheralNumber);
    
    // Check if the device file exists and is accessible
    if (_devicePath.empty()) {
        return ErrorType::InvalidParameter;
    }
    
    // For now, we'll assume the device exists if we have a valid path
    // In a real implementation, you'd want to check if the file exists
    return ErrorType::Success;
}