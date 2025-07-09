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

    /// @brief Overrun interrupt.
    constexpr uint32_t OverrunInterrupt = 0x01;
    /// @brief Break error interrupt.
    constexpr uint32_t BreakErrorInterrupt = 0x02;
    /// @brief Parity error interrupt.
    constexpr uint32_t ParityErrorInterrupt = 0x04;
    /// @brief Framing error interrupt.
    constexpr uint32_t FramingErrorInterrupt = 0x08;
    /// @brief Receive timeout interrupt.
    constexpr uint32_t ReceiveTimeoutInterrupt = 0x10;
    /// @brief Transmitted interrupt.
    constexpr uint32_t TransmittedInterrupt = 0x20;
    /// @brief Received interrupt.
    constexpr uint32_t ReceivedInterrupt = 0x40;
    /// @brief Clear to send modem interrupt.
    constexpr uint32_t CtsModemInterrupt = 0x80;
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
    /// @brief When a pin is unused, set it to this value
    static constexpr PinNumber Unused = -1;

    /**
     * @brief Set hardware configuration parameters.
     * @param [in] txNumber The transmit pin number.
     * @param [in] rxNumber The receive pin number.
     * @param [in] rtsNumber The RTS pin number.
     * @param [in] ctsNumber The CTS pin number.
     * @param [in] peripheralNumber The UART peripheral number.
     * @returns ErrorType::Success if the hardware configuration was set successfully.
     * @returns ErrorType::InvalidParameter if the txNumber, rxNumber, rtsNumber, ctsNumber, or peripheralNumber are invalid for the underlying hardware implementation.
    */
    virtual ErrorType setHardwareConfig(PinNumber txNumber, PinNumber rxNumber, PinNumber rtsNumber, PinNumber ctsNumber, PeripheralNumber peripheralNumber) = 0;
    /**
     * @brief Set driver configuration parameters.
     * @param [in] baudRate The baud rate.
     * @param [in] dataBits The data bits.
     * @param [in] parity The parity bit.
     * @param [in] stopBits The stop bits.
     * @param [in] flowControl The flow control.
     * @returns ErrorType::Success if the driver configuration was set successfully.
     * @returns ErrorType::InvalidParameter if the baud rate, data bits, parity, stop bits, or flow control are invalid for the underlying hardware implementation.
    */
    virtual ErrorType setDriverConfig(uint32_t baudRate, uint8_t dataBits, char parity, uint8_t stopBits, UartTypes::FlowControl flowControl) = 0;
    /**
     * @brief Set firmware configuration parameters.
     * @param [in] receiveBufferSize The receive buffer size.
     * @param [in] transmitBufferSize The transmit buffer size.
     * @param [in] terminatingByte The terminating byte. Used for when you want to receive bytes until a certain character is read.
     * @return Always returns ErrorType::Success.
    */
    virtual ErrorType setFirmwareConfig(Bytes receiveBufferSize, Bytes transmitBufferSize, int8_t terminatingByte) = 0;
    /**
     * @brief Set interrupt configuration parameters.
     * @param[in] interruptFlags The interrupt flags.
     * @see UartTypes::OverrunInterrupt, UartTypes::BreakErrorInterrupt, UartTypes::ParityErrorInterrupt, UartTypes::FramingErrorInterrupt, UartTypes::ReceiveTimeoutInterrupt, UartTypes::TransmittedInterrupt, UartTypes::ReceivedInterrupt, UartTypes::CtsModemInterrupt
     * @param[in] interruptCallback The interrupt callback function.
     * @returns ErrorType::Success if the interrupt configuration was set successfully.
     * @returns ErrorType::Failure otherwise.
    */
    virtual ErrorType setInterruptConfig(InterruptFlags interruptFlags, InterruptCallback interruptCallback) = 0;

    /// @brief Get the baud rate.
    uint32_t baudRate() const { return _baudRate; }
    /// @brief Get the data bits.
    uint32_t dataBits() const { return _dataBits; }
    /// @brief Get the stop bits.
    uint32_t stopBits() const { return _stopBits; }
    /// @brief Get the parity.
    char parity() const { return _parity; }
    /// @brief Get the flow control.
    UartTypes::FlowControl flowControl() const { return _flowControl; }
    /// @brief Get the tx pin number.
    PinNumber txNumber() const { return _txNumber; }
    /// @brief Get the rx pin number.
    PinNumber rxNumber() const { return _rxNumber; }
    /// @brief Get the rts pin number
    PinNumber rtsNumber() const { return _rtsNumber; }
    /// @brief Get the cts pin number
    PinNumber ctsNumber() const { return _ctsNumber; }
    /// @brief Get the peripheral number.
    PeripheralNumber peripheralNumber() const { return _peripheralNumber; }
    /// @brief Get the receive buffer size.
    Bytes receiveBufferSize() const { return _receiveBufferSize; }
    /// @brief Get the transmit buffer size.
    Bytes transmitBufferSize() const { return _transmitBufferSize; }
    /// @brief Get the terminating byte.
    int8_t terminatingByte() const { return _terminatingByte; }

    protected:
    /// @brief Baud rate.
    uint32_t _baudRate = 115200;
    /// @brief Data bits.
    uint8_t _dataBits = 8;
    /// @brief Parity.
    char _parity = 'N';
    /// @brief Stop bits.
    uint8_t _stopBits = 1;
    /// @brief Flow control.
    UartTypes::FlowControl _flowControl = UartTypes::FlowControl::Disable;
    /// @brief Terminating byte. Default to not used.
    int8_t _terminatingByte = -1;

    /// @brief Peripheral number.
    PeripheralNumber _peripheralNumber = PeripheralNumber::Unknown;
    /// @brief tx pin number.
    PinNumber _txNumber = Unused;
    /// @brief rx pin number
    PinNumber _rxNumber = Unused;
    /// @brief rts pin number
    PinNumber _rtsNumber = Unused;
    /// @brief cts pin number
    PinNumber _ctsNumber = Unused;
    /// @brief Receive buffer size.
    Bytes _receiveBufferSize = 0;
    /// @brief Transmit buffer size.
    Bytes _transmitBufferSize = 0;
    /// @brief Interrupt flags.
    InterruptFlags _interruptFlags = 0;
    /// @brief Interrupt callback.
    InterruptCallback _interruptCallback;
};

#endif // __UART_ABSTRACTION_HPP__
