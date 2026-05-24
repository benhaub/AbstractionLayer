//AbstractionLayer
#include "SpiModule.hpp"
#include "Log.hpp"

namespace {

    /**
     * @struct VendorProductId
     * @brief A vendor/product for each peripheral number initialized.
     * @note Plug in your ftdi device and use dmesg to find the vendor and product IDs.
     */
    struct VendorProductId {
        uint16_t vendorId = 0;
        uint16_t productId = 0;
        PeripheralNumber peripheralNumber = PeripheralNumber::Unknown;
    };
}

#ifndef SPI_LIBMPSSE_VENDOR_PRODUCT_IDS
#error "Define an initializer list of vendor/product ID pairs"
#endif

ErrorType Spi::init() {
    ErrorType error = ErrorType::InvalidParameter;
    ChannelConfig channelConf;
    const bool parametersAreValid = PeripheralNumber::Unknown != spiParams().hardwareConfig.peripheral &&
                                    //AN_178 LibMPSEE User Guide. Sect. 3.4.1, Pg. 12
                                    spiParams().driverConfig.clockFrequency <= 30E6 &&
                                    SpiTypes::ChipSelectMode::Hardware == spiParams().hardwareConfig.chipSelectMode;

    if (parametersAreValid) {
        static constexpr VendorProductId vendorProductIds[] = SPI_LIBMPSSE_VENDOR_PRODUCT_IDS;
        FT_STATUS status = FT_OK;

        for (const VendorProductId &vendorProductId : vendorProductIds) {

            if (vendorProductId.peripheralNumber == spiParams().hardwareConfig.peripheral) {
                status = FT_SetVIDPID(vendorProductId.vendorId, vendorProductId.productId);
                break;
            }
        }

        if (FT_OK == status) {
            uint32_t channels = 0;
            status = SPI_GetNumChannels(&channels);

            if (FT_OK == status) {
                const uint32_t channel = toLibMpsseChannelNumber(spiParams().hardwareConfig.peripheral, error);

                if (ErrorType::Success == error) {

                    if (channel < channels) {
                        status = SPI_OpenChannel(channel, &_handle);

                        if (FT_OK == status) {
                            FT_DEVICE_LIST_INFO_NODE devList;
                            status = SPI_GetChannelInfo(channel, &devList);

                            if (FT_OK == status) {
                                PLT_LOGI(TAG, "Channel opened <Description:%s, SerialNumber:%s, VendorID:0x%x, ProductId:0x%x>", devList.Description, devList.SerialNumber, (devList.ID & 0xFFFF0000) >> 16, devList.ID & 0x0000FFFF);
                            }

                            channelConf.ClockRate = spiParams().driverConfig.clockFrequency;
                            channelConf.LatencyTimer = 1;
                            channelConf.configOptions = toLibMpsseConfigOptions(error);
                            channelConf.Pin = 0;

                            status = SPI_InitChannel(_handle, &channelConf);

                            if (FT_OK == status) {
                                error = ErrorType::Success;
                            }
                        }
                    }
                    else {
                        error = ErrorType::InvalidParameter;
                    }
                }
            }
        }
        else {
            error = ErrorType::Failure;
        }
    }

    return error;
}

ErrorType Spi::deinit() {
    SPI_CloseChannel(_handle);
    Cleanup_libMPSSE();
    _handle = nullptr;
    return ErrorType::Success;
}

ErrorType Spi::txBlocking(const StaticString::Container &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return txBlocking(data->c_str(), data->size(), timeout, params);
}
ErrorType Spi::txBlocking(std::string_view data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return txBlocking(data.data(), data.size(), timeout, params);
}
ErrorType Spi::txBlocking(const char *data, const size_t size, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    ErrorType error = ErrorType::InvalidParameter;

    if (IcCommunicationProtocolTypes::IcDevice::Spi == params.deviceType()) {
        const auto &additionalParams = static_cast<const SpiTypes::AdditionalCommunicationParameters &>(params);
        Bytes transferred = 0;
        UCHAR *ftData = reinterpret_cast<UCHAR *>(const_cast<char *>(data));
        const uint32_t options = toMpsseSpiReadWriteOptions(additionalParams.chipSelectControl, error);

        if (ErrorType::Success == error) {
            FT_STATUS status = SPI_Write(_handle, ftData, size, &transferred, options);
            return FT_OK == status ? ErrorType::Success : ErrorType::Failure;
        }
    }

    return error;
}

ErrorType Spi::rxBlocking(StaticString::Container &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    size_t read = 0;
    const ErrorType error = rxBlocking(buffer->data(), buffer->size(), read, timeout, params);

    if (ErrorType::Success == error) {
        buffer->resize(read);
    }

    return error;
}
ErrorType Spi::rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    size_t read = 0;
    const ErrorType error = rxBlocking(buffer.data(), buffer.size(), read, timeout, params);

    if (ErrorType::Success == error) {
        buffer.resize(read);
    }

    return error;
}
ErrorType Spi::rxBlocking(char *buffer, const size_t bufferSize, size_t &bytesRead, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    ErrorType error = ErrorType::InvalidParameter;

    if (IcCommunicationProtocolTypes::IcDevice::Spi == params.deviceType()) {
        const auto &additionalParams = static_cast<const SpiTypes::AdditionalCommunicationParameters &>(params);
        uint32_t sizeTransferred = 0;
        const uint32_t options = toMpsseSpiReadWriteOptions(additionalParams.chipSelectControl, error);

        if (ErrorType::Success == error) {
            FT_STATUS status = SPI_Read(_handle, reinterpret_cast<UCHAR *>(buffer), bufferSize, &sizeTransferred, options);

            if (FT_OK == status) {
                bytesRead = sizeTransferred;
                error = ErrorType::Success;
            }
            else {
                error = ErrorType::Failure;
            }
        }
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