//AbstractionLayer
#include "SpiModule.hpp"
#include "PowerResetClockManagementAbstraction.hpp"
//TI Driverlib
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

ErrorType Spi::init() {
    ErrorType error = ErrorType::PrerequisitesNotMet;

    if (PeripheralNumber::Unknown != spiParams().hardwareConfig.peripheral) {
        const uint32_t tm4cSpiSysCtlPeripheralNumber = toTm4cSysCtlPeripheralNumber(spiParams().hardwareConfig.peripheral, error);

        if (ErrorType::Success == error) {
            SysCtlPeripheralEnable(tm4cSpiSysCtlPeripheralNumber);

            while(!SysCtlPeripheralReady(tm4cSpiSysCtlPeripheralNumber));

            const Register tm4cSpiBaseRegister = toTm4cPeripheralBaseRegister(spiParams().hardwareConfig.peripheral, error);

            if (ErrorType::Success == error) { 
                const uint32_t format = toTm4cFrameFormat(spiParams().driverConfig.format, error);

                if (ErrorType::Success == error) {
                    const uint32_t mode = spiParams().driverConfig.isController ? SSI_MODE_MASTER : SSI_MODE_SLAVE;
                    const Hertz frequency = spiParams().driverConfig.clockFrequency;

                    if (frequency <= 60E6 && frequency >= 2E6) {
                        const uint32_t dataSize = toTm4cDataSize(spiParams().driverConfig.dataSize, error);

                        if (ErrorType::Success == error) {
#if defined(PART_TM4C1294NCPDT)
                            SSIConfigSetExpClk(reinterpret_cast<uint32_t>(tm4cSpiBaseRegister), PowerResetClockManagementTypes::SystemClockFrequency, format, mode, frequency, dataSize);
#elif defined(PART_TM4C123GH6PM)
                            SSIConfigSetExpClk(reinterpret_cast<uint32_t>(tm4cSpiBaseRegister), SysCtlClockGet(), format, mode, frequency, dataSize);
#endif
                            SSIEnable(reinterpret_cast<uint32_t>(tm4cSpiBaseRegister));
                        }
                    }
                    else {
                        error = ErrorType::InvalidParameter;
                    }
                }
            }
        }
    }

    return error;
}

ErrorType Spi::deinit() {
    ErrorType error;
    const Register tm4cSpiBaseRegister = toTm4cPeripheralBaseRegister(spiParams().hardwareConfig.peripheral, error);

    if (ErrorType::Success == error) {
        SSIDisable(reinterpret_cast<uint32_t>(tm4cSpiBaseRegister));
    }

    return error;
}

ErrorType Spi::txBlocking(const StaticString::Container &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return txBlocking(data->c_str(), data->size(), timeout);
}
ErrorType Spi::txBlocking(std::string_view data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    return txBlocking(data.data(), data.size(), timeout);
}
ErrorType Spi::txBlocking(const char *data, const size_t size, const Milliseconds timeout) {
    ErrorType error;
    const Register baseAddress = toTm4cPeripheralBaseRegister(spiParams().hardwareConfig.peripheral, error);

    if (ErrorType::Success == error) {
        for (size_t i = 0; i < size; i++) {
            SSIDataPut(reinterpret_cast<uint32_t>(baseAddress), data[i]);
        }
    }

    while (SSIBusy(reinterpret_cast<uint32_t>(baseAddress)));

    return error;
}

ErrorType Spi::rxBlocking(StaticString::Container &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    size_t bytesRead = 0;
    ErrorType error;

    error = rxBlocking(buffer->data(), buffer->size(), bytesRead, timeout);

    if (ErrorType::Success == error) {
        buffer->resize(bytesRead);
    }

    return error;
}
ErrorType Spi::rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) {
    size_t bytesRead = 0;
    ErrorType error;

    error = rxBlocking(buffer.data(), buffer.size(), bytesRead, timeout);

    if (ErrorType::Success == error) {
        buffer.resize(bytesRead);
    }

    return error;
}
ErrorType Spi::rxBlocking(char *buffer, const size_t bufferSize, size_t &bytesRead, const Milliseconds timeout) {
    ErrorType error;
    uint32_t receivedByte;
    const Register baseAddress = toTm4cPeripheralBaseRegister(spiParams().hardwareConfig.peripheral, error);

    bytesRead = 0;

    if (ErrorType::Success == error) {

        for (size_t i = 0; i < bufferSize; i++) {
            SSIDataGet(reinterpret_cast<uint32_t>(baseAddress), &receivedByte);
            buffer[i] = receivedByte & 0xFF;
            bytesRead++;
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
    ErrorType error;
    uint32_t receivedByte;
    const Register baseAddress = toTm4cPeripheralBaseRegister(spiParams().hardwareConfig.peripheral, error);

    if (ErrorType::Success == error) {

        while (SSIDataGetNonBlocking(reinterpret_cast<uint32_t>(baseAddress), &receivedByte));
    }

    return error;
}

uint32_t Spi::toTm4cSysCtlPeripheralNumber(PeripheralNumber peripheralNumber, ErrorType &error) {
    error = ErrorType::Success;

    switch (peripheralNumber) {
        case PeripheralNumber::Zero:
            return SYSCTL_PERIPH_SSI0;
        case PeripheralNumber::One:
            return SYSCTL_PERIPH_SSI1;
        case PeripheralNumber::Two:
            return SYSCTL_PERIPH_SSI2;
        case PeripheralNumber::Three:
            return SYSCTL_PERIPH_SSI3;
        default:
            error = ErrorType::InvalidParameter;
            return 0;
    }
}

Register Spi::toTm4cPeripheralBaseRegister(PeripheralNumber peripheralNumber, ErrorType &error) {
    error = ErrorType::Success;

    switch (peripheralNumber) {
        case PeripheralNumber::Zero:
            return Register(SSI0_BASE);
        case PeripheralNumber::One:
            return Register(SSI1_BASE);
        case PeripheralNumber::Two:
            return Register(SSI2_BASE);
        case PeripheralNumber::Three:
            return Register(SSI3_BASE);
        default:
            error = ErrorType::InvalidParameter;
            return 0;
    }
}

uint32_t Spi::toTm4cFrameFormat(const SpiTypes::FrameFormat format, ErrorType &error) {
    error = ErrorType::Success;

    switch(format) {
        case SpiTypes::FrameFormat::Mode0:
            return SSI_FRF_MOTO_MODE_0;
        case SpiTypes::FrameFormat::Mode1:
            return SSI_FRF_MOTO_MODE_1;
        case SpiTypes::FrameFormat::Mode2:
            return SSI_FRF_MOTO_MODE_2;
        case SpiTypes::FrameFormat::Mode3:
            return SSI_FRF_MOTO_MODE_3;
        default:
            error = ErrorType::InvalidParameter;
            return SSI_FRF_MOTO_MODE_0;
    }
}

uint32_t Spi::toTm4cDataSize(const SpiTypes::DataSize dataSize, ErrorType &error) {
    error = ErrorType::Success;

    switch (dataSize) {
        case SpiTypes::DataSize::FourBits:
            return 0x4;
        case SpiTypes::DataSize::FiveBits:
            return 0x5;
        case SpiTypes::DataSize::SixBits:
            return 0x6;
        case SpiTypes::DataSize::SevenBits:
            return 0x7;
        case SpiTypes::DataSize::EightBits:
            return 0x8;
        case SpiTypes::DataSize::NineBits:
            return 0x9;
        case SpiTypes::DataSize::TenBits:
            return 0xa;
        case SpiTypes::DataSize::ElevenBits:
            return 0xb;
        case SpiTypes::DataSize::TwelveBits:
            return 0xc;
        case SpiTypes::DataSize::ThirteenBits:
            return 0xd;
        case SpiTypes::DataSize::FourteenBits:
            return 0xe;
        case SpiTypes::DataSize::FifteenBits:
            return 0xf;
        case SpiTypes::DataSize::SixteenBits:
            return 0x10;
        default:
            error = ErrorType::InvalidParameter;
            return 0x0;
    }
}