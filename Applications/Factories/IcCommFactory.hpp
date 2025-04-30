/***************************************************************************//**
* @author   Ben Haubrich
* @file     IcCommFactory.hpp
* @details  Factory for creating various types of IC communication protocols
* @ingroup Applications
*******************************************************************************/
#ifndef __IC_COMM_FACTORY_HPP__
#define __IC_COMM_FACTORY_HPP__

//AbstractionLayer
#include "Types.hpp"
#include "Error.hpp"
#include "SpiModule.hpp"
#include "UartModule.hpp"
#include "I2cModule.hpp"
#include "Log.hpp"
//C++
#include <cassert>

///@brief The tag used for logging
static constexpr char IcCommFactoryTag[] = "IcCommFactory";

/**
 * @namespace NetworkFactoryTypes
 * @brief Contains types and constants used by the NetworkFactory.
 */
namespace IcCommFactoryTypes {

    /**
     * @brief Parameters to pass to the factory.
     */
    struct FactoryParams {
        public:
        virtual IcCommunicationProtocolTypes::IcDevice deviceType() const = 0;
    };

    /**
     * @struct UartParams
     * @brief Contains the parameters used to configure the uart.
     */
    struct UartParams final : public FactoryParams {
        public:
        IcCommunicationProtocolTypes::IcDevice deviceType() const override { return IcCommunicationProtocolTypes::IcDevice::Uart; }

        struct hardwareConfig {
            UartTypes::Line line;
            PinNumber tx;
            PinNumber rx;
            PinNumber rts;
            PinNumber cts;
            PeripheralNumber peripheralNumber;
        } hardwareConfig;
        struct driverConfig {
            uint32_t baudRate;
            uint8_t dataBits;
            char parity;
            uint8_t stopBits;
            UartTypes::FlowControl flowControl;
        } driverConfig;
        struct firmwareConfig {
            Bytes receiveBufferSize;
            Bytes transmitBufferSize;
            int8_t terminatingByte;
        } firmwareConfig;
        struct interruptConfig {
            InterruptFlags interruptFlags;
            InterruptCallback interruptCallback;
        } interruptConfig;
    };

    struct SpiParams final : public FactoryParams {
        public:
        IcCommunicationProtocolTypes::IcDevice deviceType() const override { return IcCommunicationProtocolTypes::IcDevice::Spi; }
    };

    struct I2cParams final : public FactoryParams {
        public:
        IcCommunicationProtocolTypes::IcDevice deviceType() const override { return IcCommunicationProtocolTypes::IcDevice::I2c; }
    };
}

//Anonymous namespace.
namespace {
    ErrorType UartConfigure(Uart &uart, const IcCommFactoryTypes::UartParams &params) {
        ErrorType error = uart.setHardwareConfig(
            params.hardwareConfig.tx, 
            params.hardwareConfig.rx, 
            params.hardwareConfig.rts,
            params.hardwareConfig.cts,
            params.hardwareConfig.peripheralNumber
        );
        if (ErrorType::NotImplemented == error) {
            return ErrorType::NotImplemented;
        }
        else if (ErrorType::NotAvailable == error) {
            PLT_LOGW(IcCommFactoryTag, "Hardware configuration is unavailable on this platform");
        }

        error = uart.setDriverConfig(
            params.driverConfig.baudRate,
            params.driverConfig.dataBits,
            params.driverConfig.parity,
            params.driverConfig.stopBits,
            params.driverConfig.flowControl
        );
        if (ErrorType::NotImplemented == error) {
            return ErrorType::NotImplemented;
        }
        else if (ErrorType::NotAvailable == error) {
            PLT_LOGW(IcCommFactoryTag, "Driver configuration is unavailable on this platform");
        }

        error = uart.setFirmwareConfig(
            params.firmwareConfig.receiveBufferSize,
            params.firmwareConfig.transmitBufferSize,
            params.firmwareConfig.terminatingByte
        );
        if (ErrorType::NotImplemented == error) {
            return ErrorType::NotImplemented;
        }
        else if (ErrorType::NotAvailable == error) {
            PLT_LOGW(IcCommFactoryTag, "Firmware configuration is unavailable on this platform");
        }

        return error;
    }

    ErrorType SpiConfigure(Spi &spi) {
        ErrorType error = ErrorType::NotImplemented;
        return error;
    }

    ErrorType I2cConfigure(I2c &i2c) {
        ErrorType error = ErrorType::NotImplemented;
        return error;
    }
}

/**
 * @namespace NetworkFactory
 * @brief Functions and types that aid the creation of network interfaces.
 */
namespace IcCommFactory {

    /**
     * @brief Creates a IC device for the type selected.
     * @param device The IC device to use for communication.
     * @sa IcCommunicationProtocolTypes::IcDevice
     * @param error The error code following the return of this function.
     * @returns Pointer to an IcCommunicationProtocol that contains the IC device of the type selected.
     */
    inline std::unique_ptr<IcCommunicationProtocol> Factory(IcCommunicationProtocolTypes::IcDevice device, const IcCommFactoryTypes::FactoryParams &params, ErrorType &error) {
        error = ErrorType::Success;

        switch (device) {
        case IcCommunicationProtocolTypes::IcDevice::Uart: {
            auto uart = std::make_unique<Uart>();
            assert(nullptr != uart.get());
            assert(params.deviceType() == IcCommunicationProtocolTypes::IcDevice::Uart);

            error = UartConfigure(*(uart.get()), static_cast<const IcCommFactoryTypes::UartParams &>(params));
            if (ErrorType::Success != error) {
                const bool isCriticalError = !(ErrorType::NotAvailable == error);
                if (isCriticalError) {
                    PLT_LOGE(IcCommFactoryTag, "Uart configuration failed <error:%u>", (uint8_t)error);
                    return nullptr;
                }
            }

            error = uart->init();
            //If NotAvailable is returned it just means that initializing Wifi doesn't make sense on the current platform. For desktop
            //platforms the wifi is proabably already initialized for you.
            const bool isCriticalError = !(ErrorType::NotAvailable == error);
            if (ErrorType::Success != error) {
                if (isCriticalError) {
                    PLT_LOGE(IcCommFactoryTag, "Failed to initialize uart <error:%u>", (uint8_t)error);
                    return nullptr;
                }
            }

            if (ErrorType::Success == error || !isCriticalError) {
                PLT_LOGI(IcCommFactoryTag, "Uart initialized <Peripheral:%u, Tx pin:%d, Rx pin:%d, Baud rate:%u, Data bits:%u, Parity:%c, Stop bits:%u,\
                                             Flow control:%u, Receive buffer size:%u, Transmit buffer size:%u, Terminating byte:%u>",
                    (uint8_t)uart->peripheralNumber(), uart->txNumber(), uart->rxNumber(), uart->baudRate(), uart->dataBits(), uart->parity(), uart->stopBits(), (uint8_t)uart->flowControl(), uart->receiveBufferSize(), uart->transmitBufferSize(), uart->terminatingByte());
                return uart;
            }

            break;
        }
        case IcCommunicationProtocolTypes::IcDevice::Spi: {
            auto spi = std::make_unique<Spi>();
            assert(nullptr != spi.get());

            error = SpiConfigure(*(spi.get()));
            break;
        }
        case IcCommunicationProtocolTypes::IcDevice::I2c: {
            auto i2c = std::make_unique<I2c>();
            assert(nullptr != i2c.get());

            error = I2cConfigure(*(i2c.get()));
            break;
        }
        case IcCommunicationProtocolTypes::IcDevice::Ethernet:
        case IcCommunicationProtocolTypes::IcDevice::Usb:
        default:
            error = ErrorType::NotSupported;
            break;
        }

        return nullptr;
    }
}

#endif // __NETWORK_FACTORY_HPP__