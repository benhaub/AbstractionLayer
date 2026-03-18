/***************************************************************************//**
* @author   Ben Haubrich
* @file     I2cAbstraction.hpp
* @details  Abstraction layer for I2C communication.
* @sa       <https://www.nxp.com/docs/en/user-guide/UM10204.pdf>
* @ingroup  Abstractions
*******************************************************************************/
#ifndef __I2C_ABSTRACTION_HPP__
#define __I2C_ABSTRACTION_HPP__

//AbstractionLayer
#include "IcCommunicationProtocol.hpp"

/**
 * @namespace I2cTypes
 * @brief Types for the I2C driver.
 */
namespace I2cTypes {
    /**
     * @enum Mode
     * @details I2C mode
     * @sa User manual Pg.6
     */
    enum class Mode : uint8_t {
        Unknown = 0, ///< Unknown mode
        Controller,  ///< Device that initiates a transfer, generates clock signals and terminates a transfer
        Target,      ///< The device that is addressed by the controller.
    };

    /**
     * @enum Speed
     * @details I2C speed in bits per second
     */
    enum class Speed : Hertz {
        Unknown = 0,          ///< Unknown speed
        Standard = 100000,    ///< Standard speed (100 kbps)
        Fast = 400000,        ///< Fast speed (400 kbps)
        FastPlus = 1000000,   ///< Fast Plus speed (1000 kbps)
        HighSpeed = 3400000,  ///< High speed (3.4 Mbps)
        UltraFast = 5000000,  ///< Ultra-fast speed (5 Mbps)
    };

    /**
     * @struct I2cParams
     * @brief Contains the parameters used to configure I2C.
     */
    struct I2cParams final : public IcCommunicationProtocolTypes::ConfigurationParameters {
        IcCommunicationProtocolTypes::IcDevice deviceType() const override { return IcCommunicationProtocolTypes::IcDevice::I2c; }

        /**
         * @struct HardwareConfig
         * @brief Contains the hardware configuration parameters for the I2C.
         */
        struct HardwareConfig {
            PeripheralNumber peripheral; ///< The hardware peripheral number
            I2cTypes::Mode mode; ///< The mode of the I2C.
            I2cTypes::Speed speed; ///< The speed of the I2C.
            PinNumber sda; ///< The pin number for the SDA line.
            bool sdaPullup; ///< Whether the SDA line should have a pull-up resistor.
            PinNumber scl; ///< The pin number for the SCL line.
            bool sclPullup; ///< Whether the SCL line should have a pull-up resistor.
        } hardwareConfig; ///< The hardware configuration parameters.

        /**
         * @struct InterruptConfig
         * @brief Contains the interrupt configuration parameters for the I2C.
         */
        struct InterruptConfig {
            bool arbitrationLost; ///< Whether the arbitration was lost.
            bool nackDetected; ///< Whether the target did not acknowledge the transfer.
            bool sclLowTimeout; ///< Whether the controller has lost clock synchronization with another controller because the SCL line was held low longer than the configured timeout.
            bool stopDetect; ///< Whether the controller has finished sending data. The stop bit is sent and this interrupt is activated.
            bool receiveFifoOverflow; ///< Whether the receive FIFO has reached a threshold.
            bool transmitFifoOverflow; ///< Whether the transmit FIFO has reached a threshold.
        } interruptConfig; ///< The interrupt configuration parameters.

        /// @brief Constructor.
        I2cParams() : IcCommunicationProtocolTypes::ConfigurationParameters() {
            hardwareConfig.mode = I2cTypes::Mode::Unknown;
            hardwareConfig.speed = I2cTypes::Speed::Unknown;
            hardwareConfig.peripheral = PeripheralNumber::Unknown;
            hardwareConfig.sda = -1;
            hardwareConfig.sdaPullup = false;
            hardwareConfig.scl = -1;
            hardwareConfig.sclPullup = false;

            interruptConfig.arbitrationLost = false;
            interruptConfig.nackDetected = false;
            interruptConfig.sclLowTimeout = false;
            interruptConfig.stopDetect = false;
            interruptConfig.receiveFifoOverflow = false;
            interruptConfig.transmitFifoOverflow = false;
        }
    };
}

/**
 * @class I2cAbstraction
 * @brief Abstraction layer for I2C communication.
 * @details The interface provided by IcCommunication does not have all the parameters we need, but we still want to maintain the polymorphism so the tx and rx family of
 *          functions have been redefined for this interface. You will need to cast to an I2C abstraction if you store this class polymorphically.
 * @note All member functions can return any of ErrorType::NotImplemented, ErrorType::NotSupported or ErrorType::NotAvailable.
 */
class I2cAbstraction : public IcCommunicationProtocol {

    public:
    /// @brief Constructor.
    I2cAbstraction() : IcCommunicationProtocol() {}
    /// @brief Destructor.
    virtual ~I2cAbstraction() = default;

    /// @brief The tag for logging
    static constexpr char TAG[] = "I2c";

    ErrorType configure(const IcCommunicationProtocolTypes::ConfigurationParameters &params) override {
        _i2cParams = static_cast<const I2cTypes::I2cParams &>(params);

        return ErrorType::NotImplemented;
    }

    /// @brief Get the I2C parameters as a constant reference.
    const I2cTypes::I2cParams &i2cParams() const { return _i2cParams; }

    private:
    /// @brief The I2C parameters.
    I2cTypes::I2cParams _i2cParams;
};

#endif //__I2C_ABSTRACTION_HPP__