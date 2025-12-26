/***************************************************************************//**
* @author  Ben Haubrich
* @file    UartAbstraction.hpp
* @details Abstraction for UART communication.
* @ingroup Abstractions
*******************************************************************************/
#ifndef __UART_ABSTRACTION_HPP__
#define __UART_ABSTRACTION_HPP__

//AbstractionLayer
#include "IcCommunicationProtocol.hpp"

/**
 * @namespace UartTypes
 * @brief Uart configuration types
*/
namespace UartTypes {

    /**
     * @enum FlowControl
     * @brief   UART flow control.
    */
    enum class FlowControl : uint8_t {
        Unknown = 0,
        Disable, ///< Disable flow control.
        Rts,     ///< Enable RTS flow control.
        Cts,     ///< Enable CTS flow control.
        CtsRts   ///< Enable both RTS and CTS flow control.
    };

    /**
     * @enum Line
     * @brief UART transmit and receive lines.
    */
    enum class Line : uint8_t {
        Unknown = 0, ///< Unknown and invalid line.
        Tx,          ///< Transmit line.
        Rx,          ///< Receive line.
        Both         ///< Transmit and receive line.
    };

    namespace Interrupts {
        /// @brief Overrun interrupt.
        constexpr InterruptFlags OverrunInterrupt = 0x01;
        /// @brief Break error interrupt.
        constexpr InterruptFlags BreakErrorInterrupt = 0x02;
        /// @brief Parity error interrupt.
        constexpr InterruptFlags ParityErrorInterrupt = 0x04;
        /// @brief Framing error interrupt.
        constexpr InterruptFlags FramingErrorInterrupt = 0x08;
        /// @brief Receive timeout interrupt.
        constexpr InterruptFlags ReceiveTimeoutInterrupt = 0x10;
        /// @brief Transmitted interrupt.
        constexpr InterruptFlags TransmittedInterrupt = 0x20;
        /// @brief Received interrupt.
        constexpr InterruptFlags ReceivedInterrupt = 0x40;
        /// @brief Clear to send modem interrupt.
        constexpr InterruptFlags CtsModemInterrupt = 0x80;
        /// Interrupts are disabled
        constexpr InterruptFlags Disabled = 0x00;
    }

    /**
     * @struct UartParams
     * @brief Contains the parameters used to configure the UART.
     */
    struct UartParams final : public IcCommunicationProtocolTypes::ConfigurationParameters {
        IcCommunicationProtocolTypes::IcDevice deviceType() const override { return IcCommunicationProtocolTypes::IcDevice::Uart; }

        /**
         * @struct HardwareConfig
         * @brief Contains the hardware configuration for the UART.
         */
        struct HardwareConfig {
            UartTypes::Line line;              ///< The line to use for the uart
            PinNumber tx;                      ///< The tx pin to use for the uart
            PinNumber rx;                      ///< The rx pin to use for the uart
            PinNumber rts;                     ///< The request to send pin to use for the uart
            PinNumber cts;                     ///< The clear to send pin to use for the uart
            PeripheralNumber peripheralNumber; ///< The peripheral number to use for the uart

            HardwareConfig() : line(UartTypes::Line::Unknown), tx(-1), rx(-1), rts(-1), cts(-1), peripheralNumber(PeripheralNumber::Unknown) {}

        } hardwareConfig; ///< The hardware configuration parameters
        /**
         * @struct DriverConfig
         * @brief Contains the driver configuration for the UART.
         */
        struct DriverConfig {
            uint32_t baudRate;                  ///< The baud rate to use for the uart
            uint8_t dataBits;                   ///< The data bits to use for the uart
            char parity;                        ///< The parity to use for the uart
            uint8_t stopBits;                   ///< The stop bits to use for the uart
            UartTypes::FlowControl flowControl; ///< The flow control to use for the uart

            DriverConfig() : baudRate(0), dataBits(0), parity('N'), stopBits(0), flowControl(UartTypes::FlowControl::Unknown) {}

        } driverConfig; ///< Teh driver configuration parameters.
        /**
         * @struct FirmwareConfig
         * @brief Contains the firmware configuration for the UART.
         */
        struct FirmwareConfig {
            Bytes receiveBufferSize;  ///< The receive buffer size to use for the uart
            Bytes transmitBufferSize; ///< The transmit buffer size to use for the uart

            FirmwareConfig() : receiveBufferSize(0), transmitBufferSize(0) {}

        } firmwareConfig; ///< The firmware configuration parameters
        /**
         * @struct InterruptConfig
         * @brief Contains the interrupt configuration for the UART.
         */
        struct InterruptConfig {
            InterruptFlags interruptFlags;       ///< The interrupt flags to use for the uart
            InterruptCallback interruptCallback; ///< The interrupt callback to use for the uart

            InterruptConfig() : interruptFlags(0), interruptCallback(nullptr) {}

        } interruptConfig; ///< The interrupt configuration parameters.

        UartParams() : IcCommunicationProtocolTypes::ConfigurationParameters(), hardwareConfig(), driverConfig(), firmwareConfig(), interruptConfig() {}
    };
}

/**
 * @class UartAbstraction
 * @brief An interface for a Uart peripheral.
*/
class UartAbstraction : public IcCommunicationProtocol{

    public:
    /**
     * @brief Constructor.
     * @details The reason we don't include the parameters from all the set*Config functions in the constructor is because
     *          on some platforms (especially desktop), the configuration parameters may not apply. We also can't return
     *          error values from the constructor so if a parameter doesn't apply and we want to notify the caller with a
     *          return code then it's better to use a member function to do that.
     */
    UartAbstraction() : IcCommunicationProtocol() {}
    /// @brief Destructor.
    virtual ~UartAbstraction() = default;

    /// @brief Tag for logging.
    static constexpr char TAG[] = "Uart";

    ErrorType configure(const IcCommunicationProtocolTypes::ConfigurationParameters &params) override {
        _uartParams = static_cast<const UartTypes::UartParams &>(params);
        return ErrorType::Success;
    }

    /// @brief Get the UART parameters as a constant reference.
    const UartTypes::UartParams &uartParams() const { return _uartParams; }

    private:
    /// @brief The UART parameters.
    UartTypes::UartParams _uartParams;
};

#endif // __UART_ABSTRACTION_HPP__