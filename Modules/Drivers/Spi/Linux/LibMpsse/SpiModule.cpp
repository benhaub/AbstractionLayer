//AbstractionLayer
#include "SpiModule.hpp"

ErrorType Spi::init() {
    ErrorType error = ErrorType::PrerequisitesNotMet;
    ChannelConfig channelConf;

    if (PeripheralNumber::Unknown != spiParams().hardwareConfig.peripheral &&
        spiParams().driverConfig.clockFrequency <= 30E6) {
        uint32_t channels = 0;
        FT_STATUS status = SPI_GetNumChannels(&channels);

        if (FT_OK == status) {
            const uint32_t channel = toLibMpsseChannelNumber(spiParams().hardwareConfig.peripheral, error);

            if (ErrorType::Success == error) {

                if (channel < channels) {
                    status = SPI_OpenChannel(channel, &_handle);

                    if (FT_OK == status) {
                        channelConf.ClockRate = spiParams().driverConfig.clockFrequency;
                        channelConf.LatencyTimer = 255;
                        //DBUS3 for chip select is determined by the hardware. 3 is common but may need to be adjusted.
                        channelConf.configOptions = toLibMpsseConfigOptions(error) | SPI_CONFIG_OPTION_CS_DBUS3;

                        if (ErrorType::Success == error) {
                            status = SPI_InitChannel(_handle, &channelConf);

                            if (FT_OK == status) {
                                error = ErrorType::Success;
                            }
                        }
                    }
                }
                else {
                    error = ErrorType::InvalidParameter;
                }
            }
        }
    }

    return error;
}

ErrorType Spi::deinit() {
    SPI_CloseChannel(_handle);
    _handle = nullptr;
    return ErrorType::Success;
}

ErrorType Spi::txBlocking(const StaticString::Container &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return txBlocking(data->c_str(), data->size(), timeout);
}
ErrorType Spi::txBlocking(std::string_view data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return txBlocking(data.data(), data.size(), timeout);
}
ErrorType Spi::txBlocking(const char *data, const size_t size, const Milliseconds timeout) {
    Bytes transferred = 0;
    UCHAR *ftData = reinterpret_cast<UCHAR *>(const_cast<char *>(data));
    FT_STATUS status = SPI_Write(_handle, ftData, size, &transferred, SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);

    if (FT_OK == status) {

        BOOL busy = true;
        
        while (busy) {
            status = SPI_IsBusy(_handle, &busy);

            if (FT_OK != status) {
                break;
            }
        }
    }

    return FT_OK == status ? ErrorType::Success : ErrorType::Failure;
}

ErrorType Spi::rxBlocking(StaticString::Container &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    size_t read = 0;
    const ErrorType error = rxBlocking(buffer->data(), buffer->size(), read, timeout);

    if (ErrorType::Success == error) {
        buffer->resize(read);
        return ErrorType::Success;
    }
    else {
        return ErrorType::Failure;
    }
}
ErrorType Spi::rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    size_t read = 0;
    const ErrorType error = rxBlocking(buffer.data(), buffer.size(), read, timeout);

    if (ErrorType::Success == error) {
        buffer.resize(read);
        return ErrorType::Success;
    }
    else {
        return ErrorType::Failure;
    }
}
ErrorType Spi::rxBlocking(char *buffer, const size_t bufferSize, size_t &bytesRead, const Milliseconds timeout) {
    LPDWORD sizeTransferred = 0;
    ErrorType error = ErrorType::Failure;
    const FT_STATUS status = SPI_Read(_handle, reinterpret_cast<UCHAR *>(buffer), bufferSize, sizeTransferred, SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);

    if (FT_OK == status) {
        bytesRead = *sizeTransferred;
        error = ErrorType::Success;
    }
    else {
        error = ErrorType::Failure;
    }

    return error;
}

ErrorType Spi::txNonBlocking(const std::shared_ptr<StaticString::Container> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType Spi::txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType Spi::rxNonBlocking(std::shared_ptr<StaticString::Container> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<StaticString::Container> buffer)> callback) {
    return ErrorType::NotImplemented;
}
ErrorType Spi::rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) {
    return ErrorType::NotImplemented;
}

ErrorType Spi::flushRxBuffer() {
    return ErrorType::NotImplemented;
}