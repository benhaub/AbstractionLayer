//Modules
#include "UartModule.hpp"

ErrorType Uart::init() {
    ErrorType rxError, txError;

    txBuffer = std::make_unique<File>(Storage::Instance());
    rxBuffer = std::make_unique<File>(Storage::Instance());

    assert(nullptr != txBuffer.get());
    assert(nullptr != rxBuffer.get());
    
    std::string txFilename("/txBuffer_" + std::to_string(static_cast<uint8_t>(_peripheralNumber)));
    txError = txBuffer->open(txFilename, OpenMode::ReadWriteAppend);

    std::string rxFilename("/rxBuffer_" + std::to_string(static_cast<uint8_t>(_peripheralNumber)));
    rxError = rxBuffer->open(rxFilename, OpenMode::ReadOnly);

    if (ErrorType::Success == txError && ErrorType::Success == rxError) {
        return ErrorType::Success;
    }
    else {
        return ErrorType::FileNotFound;
    }
}

ErrorType Uart::deinit() {
    ErrorType error;
    
    error = Storage::Instance().deinitStorage();
    if (ErrorType::Success != error) {
        return error;
    }
    error = txBuffer->close();
    if (ErrorType::Success != error) {
        return error;
    }

    error = txBuffer->remove();
    if (ErrorType::Success != error) {
        return error;
    }

    return ErrorType::Success;
}

ErrorType Uart::txBlocking(const std::string &data, const Milliseconds timeout) {
    if (nullptr == txBuffer.get()) {
        return ErrorType::PrerequisitesNotMet;
    }

    Bytes written = 0;
    ErrorType error;

    if (ErrorType::Success == (error = txBuffer->writeBlocking(0, data))) {
        //Write a newline. New lines can mimic the max fifo length of the receive buffer.
        std::string newline("\n");
        if (ErrorType::Success != (error = txBuffer->writeBlocking(written, newline))) {
            return error;
        }
    }
    else {
        return error;
    }

    return ErrorType::Success;
}

ErrorType Uart::txNonBlocking(const std::shared_ptr<std::string> data, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType Uart::rxBlocking(std::string &buffer, const Milliseconds timeout) {
    ErrorType error;
    constexpr Bytes toReadAtOnce = 16;
    Bytes readInThisFrame = 0;
    std::string readBuffer(toReadAtOnce, '\0');

    if (terminatingByte() >= 0) {
        buffer.resize(0);
    }
    else {
        ErrorType error = rxBuffer->readBlocking(_totalRead, buffer);
        _totalRead += buffer.size();
        return error;
    }

    bool bufferHasRoom = readInThisFrame < buffer.capacity();

    while (bufferHasRoom) {
        error = rxBuffer->readBlocking(_totalRead, readBuffer);
        if (ErrorType::EndOfFile == error) {
            return ErrorType::Success;
        }

        _totalRead += readBuffer.size();
        readInThisFrame += readBuffer.size();

        buffer.append(readBuffer);

        if (terminatingByte() >= 0) {
            const size_t indexOfTerminatingByte = buffer.find(static_cast<char>(terminatingByte()), readInThisFrame - readBuffer.capacity());
            const bool terminatingByteFound = std::string::npos != indexOfTerminatingByte;

            if (terminatingByteFound) {
                buffer.resize(indexOfTerminatingByte + 1);
                _totalRead = indexOfTerminatingByte + 1;
                return ErrorType::Success;
            }
        }

        bufferHasRoom = readInThisFrame < buffer.capacity();
    }

    return error;
}

ErrorType Uart::rxNonBlocking(std::shared_ptr<std::string> buffer, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType Uart::flushRxBuffer() {
    return ErrorType::NotAvailable;
}

ErrorType Uart::setHardwareConfig(int32_t txPinNumber, int32_t rxPinNumber, int32_t rtsPinNumber, int32_t ctsPinNumber, PeripheralNumber peripheralNumber) {
    return ErrorType::NotAvailable;
}

ErrorType Uart::setDriverConfig(uint32_t baudRate, uint8_t dataBits, char parity, uint8_t stopBits, UartTypes::FlowControl flowControl) {
    return ErrorType::NotAvailable;
}

ErrorType Uart::setFirmwareConfig(Bytes receiveBufferSize, Bytes transmitBufferSize, int8_t terminatingByte) {
    _receiveBufferSize = receiveBufferSize;
    _transmitBufferSize = transmitBufferSize;
    _terminatingByte = terminatingByte;

    return ErrorType::Success;
}

ErrorType Uart::setInterruptConfig(InterruptFlags interruptFlags, InterruptCallback interruptCallback) {
    return ErrorType::NotImplemented;
}