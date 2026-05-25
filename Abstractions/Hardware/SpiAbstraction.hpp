/***************************************************************************//**
* @author  Ben Haubrich
* @file    SpiAbstraction.hpp
* @details Abstraction for Serial Peripheral Interface
* @ingroup Abstractions
*******************************************************************************/
#ifndef __SPI_ABSTRACTION_HPP__
#define __SPI_ABSTRACTION_HPP__

//AbstractionLayer
#include "IcCommunicationProtocol.hpp"

/**
 * @namespace SpiTypes
 * @brief Contains types and constants used by the SpiAbstraction.
 */
namespace SpiTypes {
    /**
     * @enum ChipSelectMode
     * @brief Chip select control mode
     */
    enum class ChipSelectMode : uint8_t {
        Unknown = 0,  ///< Unknown mode
        Hardware = 1, ///< Hardware SPI chip select
        Gpio = 2      ///< GPIO-based chip select
    };

    /**
     * @enum GpioChipSelectControl
     * @brief GPIO chip select control
     */
    enum class GpioChipSelectControl : uint8_t {
        Unknown = 0,        ///< Unknown mode
        Assert = 1,         ///< Assert the chip select before the current transfer
        Deassert = 2,       ///< Deassert the chip select once the current transfer is complete
        AssertDeassert = 3, ///< Assert and Deassert in one transfer
        Hold = 4            ///< Hold the chip select at its current state
    };

    /**
     * @enum FrameFormat
     * @brief The frame format for SPI
     */
    enum class FrameFormat : uint8_t {
        Unknown = 0, ///< Unknown mode
        Mode0   = 1, ///< Mode 0
        Mode1   = 2, ///< Mode 1
        Mode2   = 3, ///< Mode 2
        Mode3   = 4  ///< Mode 3
    };

    /**
     * @enum DataSize
     * @brief The number of bytes transferred on each active period
     */
    enum class DataSize : uint8_t {
        Unknown      = 0, ///< Unknown
        FourBits     = 1, ///< Transfer 4 bit each active period
        FiveBits     = 2, ///< Transfer 5 bit each active period
        SixBits      = 3, ///< Transfer 6 bit each active period
        SevenBits    = 4, ///< Transfer 7 bit each active period
        EightBits    = 5, ///< Transfer 8 bit each active period
        NineBits     = 6, ///< Transfer 9 bit each active period
        TenBits      = 7, ///< Transfer 10 bit each active period
        ElevenBits   = 8, ///< Transfer 11 bit each active period
        TwelveBits   = 9, ///< Transfer 12 bit each active period
        ThirteenBits = 10,///< Transfer 13 bit each active period
        FourteenBits = 11,///< Transfer 14 bit each active period
        FifteenBits  = 12,///< Transfer 15 bit each active period
        SixteenBits  = 13 ///< Transfer 16 bit each active period
    };

    /**
     * @enum Width
     * @brief The width of the SPI bus
     */
    enum class Channels : uint8_t {
        Unknown = 0, ///< Unknown
        Single,      ///< Single
        Dual,        ///< Dual
        Quad         ///< Quad
    };

    /**
     * @struct SpiParams
     * @brief Contains the parameters used to configure SPI.
     */
    struct SpiParams final : public IcCommunicationProtocolTypes::ConfigurationParameters {
        IcCommunicationProtocolTypes::IcDevice deviceType() const override { return IcCommunicationProtocolTypes::IcDevice::Spi; }
        SpiParams() = default;

        /**
         * @struct HardwareConfig
         * @brief Hardware configuration for SPI
         */
        struct HardwareConfig {
            PeripheralNumber peripheral = PeripheralNumber::Unknown;
            PinNumber periperhalOutControllerIn = -1; ///< POCI (MISO)
            PinNumber perpheralInControllerOut = -1;  ///< PICO (MOSI)
            PinNumber io3 = -1;                       ///< If using QSPI, set this to the third data line.
            PinNumber io4 = -1;                       ///< If using QSPI, set this to the fourth data line.
            PinNumber chipSelect = -1;                ///< CS
            PinNumber clock = -1;                     ///< CLK
            ChipSelectMode chipSelectMode = ChipSelectMode::Unknown; ///< Chip select control mode (hardware or GPIO)
            PeripheralNumber chipSelectGpioPeripheral = PeripheralNumber::Unknown; ///< GPIO chip select peripheral
            PinNumber chipSelectGpioPin = -1; ///< GPIO chip select pin
        } hardwareConfig; ///< Hardware configuration parameters

        /**
         * @struct DriverConfig
         * @brief Driver configuration for SPI
         */
        struct DriverConfig {
            bool isController = false; ///< True if the driver controls the data flow (isMaster)
            bool chipSelectActiveLow = true; ///< True if the chip select line is active low
            FrameFormat format = SpiTypes::FrameFormat::Unknown; ///< The frame format. Controls the clock and chip select during idle and active periods.
            Hertz clockFrequency = 0; ///< The frequency of the clock. Only matter for controller mode.
            DataSize dataSize = DataSize::Unknown; ///< The amount of bits sent during each active period.
            Channels channels = Channels::Unknown; ///< The width of the SPI bus
        } driverConfig; ///< Driver configuration parameters

        constexpr SpiParams(const HardwareConfig &hardwareConfig, const DriverConfig &driverConfig) : hardwareConfig(hardwareConfig), driverConfig(driverConfig) {}
    };

    /**
     * @brief Spi additional communication parameters
     */
    struct AdditionalCommunicationParameters final : public IcCommunicationProtocolTypes::AdditionalCommunicationParameters {
        /// @brief Constructor
        AdditionalCommunicationParameters() : IcCommunicationProtocolTypes::AdditionalCommunicationParameters() {}
        /// @brief Constructor
        constexpr AdditionalCommunicationParameters(SpiTypes::GpioChipSelectControl chipSelectControl) : chipSelectControl(chipSelectControl) {}

        IcCommunicationProtocolTypes::IcDevice deviceType() const override { return IcCommunicationProtocolTypes::IcDevice::Spi; }
        /// @brief The chip select control mode ofr the current transfer
        SpiTypes::GpioChipSelectControl chipSelectControl = SpiTypes::GpioChipSelectControl::Unknown;
    };
}

/**
 * @class SpiAbstraction
 * @brief Abstraction layer for Serial Peripheral Interface
*/
class SpiAbstraction : public IcCommunicationProtocol {

    public:
    ///@brief Constructor
    SpiAbstraction() : IcCommunicationProtocol() {}
    ///@brief Destructor
    virtual ~SpiAbstraction() = default;

    /// @brief Tag for logging
    static constexpr char TAG[] = "Spi";

    ErrorType configure(const IcCommunicationProtocolTypes::ConfigurationParameters &params) override {
        _spiParams = static_cast<const SpiTypes::SpiParams &>(params);
        return ErrorType::Success;
    }

    /// @brief Get the SPI parameters as a constant reference.
    const SpiTypes::SpiParams &spiParams() const { return _spiParams; }

    private:
    /// @brief The SPI parameters.
    SpiTypes::SpiParams _spiParams;
};

#endif //__GPTM_TIMER_ABSTRACTION_HPP__