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
     * @enum FrameFormat
     * @brief The frame format for SPI
     */
    enum class FrameFormat : uint8_t {
        Unknown = 0, ///< Unknown mode
        Mode0   = 1, ///< Mode 1
        Mode1   = 2, ///< Mode 2
        Mode2   = 3, ///< Mode 3
        Mode3   = 4  ///< Mode 4
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
        } hardwareConfig; ///< Hardware configuration parameters

        /**
         * @struct DriverConfig
         * @brief Driver configuration for SPI
         */
        struct DriverConfig {
            bool isController = false; ///< True if the driver controls the data flow (isMaster)
            FrameFormat format = SpiTypes::FrameFormat::Unknown; ///< The frame format. Controls the clock and chip select during idle and active periods.
            Hertz clockFrequency = 0; ///< The frequency of the clock. Only matter for controller mode.
            DataSize dataSize = DataSize::Unknown; ///< The amount of bits sent during each active period.
            Channels channels = Channels::Unknown; ///< The width of the SPI bus
        } driverConfig; ///< Driver configuration parameters
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