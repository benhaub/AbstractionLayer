#ifndef __SPI_MODULE_HPP__
#define __SPI_MODULE_HPP__

//AbstractionLayer
#include "SpiAbstraction.hpp"
//LibMPSSE
#include "ftd2xx.h"
#include "libmpsse_spi.h"


class Spi : public SpiAbstraction {
    public:
    Spi() : SpiAbstraction() {}

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType txBlocking(const StaticString::Container &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) override;
    ErrorType txBlocking(std::string_view data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) override;
    ErrorType rxBlocking(StaticString::Container &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) override;
    ErrorType rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) override;
    ErrorType txNonBlocking(const std::shared_ptr<StaticString::Container> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType rxNonBlocking(std::shared_ptr<StaticString::Container> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<StaticString::Container> buffer)> callback) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;
    ErrorType flushRxBuffer() override;

    private:
    FT_HANDLE _handle = nullptr;

    uint32_t toLibMpsseChannelNumber(const PeripheralNumber peripheralNumber, ErrorType &error) {
        error = ErrorType::Success;
        return static_cast<uint32_t>(peripheralNumber);
    }

    uint32_t toLibMpsseConfigOptions(ErrorType &error) {
        error = ErrorType::Success;
        uint32_t configuration = 0;

        switch (spiParams().driverConfig.format) {
            case SpiTypes::FrameFormat::Mode0:
                configuration |= SPI_CONFIG_OPTION_MODE0;
                break;
            case SpiTypes::FrameFormat::Mode1:
                configuration |= SPI_CONFIG_OPTION_MODE1;
                break;
            case SpiTypes::FrameFormat::Mode2:
                configuration |= SPI_CONFIG_OPTION_MODE2;
                break;
            case SpiTypes::FrameFormat::Mode3:
                configuration |= SPI_CONFIG_OPTION_MODE3;
                break;
            default:
                error = ErrorType::InvalidParameter;
                break;
        }

        if (ErrorType::Success == error) {

            spiParams().driverConfig.chipSelectActiveLow ? configuration |= SPI_CONFIG_OPTION_CS_ACTIVELOW : configuration |= SPI_CONFIG_OPTION_CS_ACTIVEHIGH;
        }

        if (ErrorType::Success == error) {

            switch (spiParams().hardwareConfig.chipSelectGpioPin) {
                case 3:
                    configuration |= SPI_CONFIG_OPTION_CS_DBUS3;
                    break;
                case 4:
                    configuration |= SPI_CONFIG_OPTION_CS_DBUS4;
                    break;
                case 5:
                    configuration |= SPI_CONFIG_OPTION_CS_DBUS5;
                    break;
                case 6:
                    configuration |= SPI_CONFIG_OPTION_CS_DBUS6;
                    break;
                case 7:
                    configuration |= SPI_CONFIG_OPTION_CS_DBUS7;
                    break;
                default:
                    error = ErrorType::InvalidParameter;
                    break;
            }
        }

        return configuration;
    }

    uint32_t toMpsseSpiReadWriteOptions(const SpiTypes::GpioChipSelectControl chipSelectControl, ErrorType &error) {
        uint32_t options = SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES;
        error = ErrorType::Success;

        if (SpiTypes::GpioChipSelectControl::Assert == chipSelectControl) {
            options |= SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE;
        }
        else if (SpiTypes::GpioChipSelectControl::Deassert == chipSelectControl) {
            options |= SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE;
        }
        else if (SpiTypes::GpioChipSelectControl::AssertDeassert == chipSelectControl) {
            options |= SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE;
        }
        else if (SpiTypes::GpioChipSelectControl::Hold == chipSelectControl) {
            return options;
        }
        else {
            error = ErrorType::InvalidParameter;
        }

        return options;
    }

    ErrorType txBlocking(const char *data, const size_t size, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params);
    ErrorType rxBlocking(char *buffer, const size_t bufferSize, size_t &bytesRead, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params);
};

#endif // __SPI_MODULE_HPP__