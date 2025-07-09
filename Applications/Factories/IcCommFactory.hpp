/***************************************************************************//**
* @author   Ben Haubrich
* @file     IcCommFactory.hpp
* @details  Factory for creating various types of IC communication protocols
* @ingroup Applications
*******************************************************************************/
#ifndef __IC_COMM_FACTORY_HPP__
#define __IC_COMM_FACTORY_HPP__

//AbstractionLayer
#include "SpiModule.hpp"
#include "UartModule.hpp"
#include "I2cModule.hpp"
#include "Log.hpp"
//C++
#include <cassert>

///@brief The tag used for logging
static constexpr char IcCommFactoryTag[] = "IcCommFactory";

/**
 * @namespace IcCommFactoryTypes
 * @brief Contains types and constants used by the IcCommFactory.
 */
namespace IcCommFactoryTypes {

    /**
     * @brief Parameters to pass to the factory.
     */
    struct FactoryParams {
        public:
        /// @brief The type of device that these parameters are for
        /// @return The deveice types
        virtual IcCommunicationProtocolTypes::IcDevice deviceType() const = 0;
    };

    /**
     * @struct UartParams
     * @brief Contains the parameters used to configure the UART.
     */
    struct UartParams final : public FactoryParams {
        public:
        IcCommunicationProtocolTypes::IcDevice deviceType() const override { return IcCommunicationProtocolTypes::IcDevice::Uart; }

        /**
         * @struct HardwareConfig
         * @brief Contains the hardware configuration for the UART.
         */
        struct HardwareConfig {
            UartTypes::Line line;               ///< The line to use for the uart
            PinNumber tx;                       ///< The tx pin to use for the uart
            PinNumber rx;                       ///< The rx pin to use for the uart
            PinNumber rts;                      ///< The request to send pin to use for the uart
            PinNumber cts;                      ///< The clear to send pin to use for the uart
            PeripheralNumber peripheralNumber;  ///< The peripheral number to use for the uart
        } hardwareConfig; ///< The hardware configuration parameters
        /**
         * @struct DriverConfig
         * @brief Contains the driver configuration for the UART.
         */
        struct DriverConfig {
            uint32_t baudRate;                   ///< The baud rate to use for the uart
            uint8_t dataBits;                    ///< The data bits to use for the uart
            char parity;                         ///< The parity to use for the uart
            uint8_t stopBits;                    ///< The stop bits to use for the uart
            UartTypes::FlowControl flowControl;  ///< The flow control to use for the uart
        } driverConfig; ///< Teh driver configuration parameters.
        /**
         * @struct FirmwareConfig
         * @brief Contains the firmware configuration for the UART.
         */
        struct FirmwareConfig {
            Bytes receiveBufferSize;             ///< The receive buffer size to use for the uart
            Bytes transmitBufferSize;            ///< The transmit buffer size to use for the uart
            int8_t terminatingByte;              ///< The terminating byte to use for the uart
        } firmwareConfig; ///< The firmware configuration parameters
        /**
         * @struct InterruptConfig
         * @brief Contains the interrupt configuration for the UART.
         */
        struct InterruptConfig {
            InterruptFlags interruptFlags;       ///< The interrupt flags to use for the uart
            InterruptCallback interruptCallback; ///< The interrupt callback to use for the uart
        } interruptConfig; ///< The interrupt configuration parameters.
    };

    /**
     * @struct SpiParams
     * @brief Contains the parameters used to configure SPI.
     */
    struct SpiParams final : public FactoryParams {
        public:
        IcCommunicationProtocolTypes::IcDevice deviceType() const override { return IcCommunicationProtocolTypes::IcDevice::Spi; }
    };

    /**
     * @struct I2cParams
     * @brief Contains the parameters used to configure I2C.
     */
    struct I2cParams final : public FactoryParams {
        public:
        IcCommunicationProtocolTypes::IcDevice deviceType() const override { return IcCommunicationProtocolTypes::IcDevice::I2c; }
    };
}

//Anonymous namespace.
namespace {
    /**
     * @brief Configures the UART.
     * @param uart The UART to configure.
     * @param params The parameters to use for the UART.
     * @returns ErrorType::Success on success
     * @returns ErrorType::NotImplemented if the UART driver is not implemented
     * @returns ErrorType::NotAvailable if UART is not available for this platform
     */
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

        error = uart.setInterruptConfig(
            params.interruptConfig.interruptFlags,
            params.interruptConfig.interruptCallback
        );
        if (ErrorType::NotImplemented == error) {
            return ErrorType::NotImplemented;
        }
        else if (ErrorType::NotAvailable == error) {
            PLT_LOGW(IcCommFactoryTag, "Interrupt configuration is unavailable on this platform");
        }

        return error;
    }

    /**
     * @brief Configures the SPI.
     * @param spi The SPI to configure.
     * @returns ErrorType::Success on success
     * @returns ErrorType::NotImplemented if the SPI driver is not implemented
     * @returns ErrorType::NotAvailable if SPI is not available for this platform
     */
    ErrorType SpiConfigure(Spi &spi) {
        ErrorType error = ErrorType::NotImplemented;
        return error;
    }

    /**
     * @brief Configures the I2C.
     * @param i2c The I2C to configure.
     * @returns ErrorType::Success on success
     * @returns ErrorType::NotImplemented if the I2C driver is not implemented
     * @returns ErrorType::NotAvailable if I2C is not available for this platform
     */
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
     * @param params The parameters to use for the IC device.
     * @sa IcCommFactoryTypes::I2cParams
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
                                             Flow control:%u, Receive buffer size:%u, Transmit buffer size:%u, Terminating byte:%d>",
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