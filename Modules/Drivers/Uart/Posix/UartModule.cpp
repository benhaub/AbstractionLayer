#include "UartModule.hpp"
//C++
#include <cstdio>
#include <cerrno>

ErrorType Uart::init() {
    ErrorType error = ErrorType::Failure;

    _devicePath = toDevicePath(uartParams().hardwareConfig.peripheralNumber);

    if (!_devicePath->empty()) {
        _fileDescriptor = open(_devicePath->c_str(), O_RDWR | O_NONBLOCK);

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
ErrorType Uart::txBlocking(const std::string &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return txBlocking(data.c_str(), data.size(), timeout);
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