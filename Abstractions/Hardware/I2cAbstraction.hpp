/***************************************************************************//**
* @author   Ben Haubrich
* @file     I2cAbstraction.hpp
* @details  Abstraction layer for I2C communication.
* @sa       <https://www.nxp.com/docs/en/user-guide/UM10204.pdf>
* @ingroup  AbstractionLayer
*******************************************************************************/
#ifndef __I2C_ABSTRACTION_HPP__
#define __I2C_ABSTRACTION_HPP__

#include "Error.hpp"
#include "Types.hpp"
#include "IcCommunicationProtocol.hpp"

namespace I2cConfig {
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
     * @enum PeripheralNumber
     * @brief   UART peripheral number. Refers to the base address of the uart module being used.
    */
    enum class PeripheralNumber : uint8_t {
        Zero = 0, ///< Peripheral number 0.
        One,      ///< Peripheral number 1.
        Two,      ///< Peripheral number 2.
        Three,    ///< Peripheral number 3.
        Four,     ///< Peripheral number 4.
        Five,     ///< Peripheral number 5.
        Unknown   ///< Unknown and invalid peripheral number.
    };
}

/**
 * @class I2cAbstraction
 * @brief Abstraction layer for I2C communication.
 * @note All member functions can return any of ErrorType::NotImplemented, ErrorType::NotSupported or ErrorType::NotAvailable.
 */
class I2cAbstraction : public IcCommunicationProtocol {
    public:
    /// @brief Constructor.
    I2cAbstraction() : IcCommunicationProtocol() {}
    /// @brief Destructor.
    virtual ~I2cAbstraction() = default;

    static constexpr char TAG[] = "I2c";

    /**
     * @brief Initialize the I2C
     * @pre Configure the I2C first.
     * @sa setHardwareConfig()
     * @sa setDriverConfig()
     * @returns ErrorType::Success if the I2C was initialized successfully.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType init() = 0;
    /**
     * @brief De-initialize the I2C
     * @returns ErrorType::Success if the I2C was de-initialized successfully.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType deinit() = 0;
    /**
     * @brief Set the hardware configurations
     * @param[in] mode The mode of the I2C.
     * @param[in] sda The pin number for the SDA line.
     * @param[in] sdaPullUp Whether the SDA line should have a pull-up resistor.
     * @param[in] scl The pin number for the SCL line.
     * @param[in] sclPullUp Whether the SCL line should have a pull-up resistor.
     * @returns ErrorType::Success if the hardware configurations were set successfully.
     * @returns ErrorType::Failure otherwise.
     * @note The reason the config is not part of the constructor parameter list is because of convenience.
     *       It's a little verbose to have sub-class constructors contain the same parameter list as the parent.
     *       It also makes cross-platform development easier since some platforms don't offer a config and having a function
     *       at least allows you to return NotSupported on NotAvailable in this case.
     */
    virtual ErrorType setHardwareConfig(const I2cConfig::PeripheralNumber peripharal, const I2cConfig::Mode mode, const I2cConfig::Speed speed, const PinNumber sda, const bool sdaPullUp, const PinNumber scl, const bool sclPullUp) = 0;

    /**
     * @brief deviceAddress
     * @returns A mutable reference the the write address
     */
    uint16_t &deviceAddress() { return _deviceAddress; }
    /**
     * @brief deviceAddressConst
     * @returns A constant reference to the read address
     */
    const uint16_t &deviceaddressConst() const { return _deviceAddress; }
    /**
     * @brief registerAddress
     * @returns A mutable reference to the register address of the device.
     */
    uint16_t &registerAddress() { return _registerAddress; }
    /*
     * @brief registerAddressConst
     * @returns A constant reference to the register address of the device.
     */
    const uint16_t &registerAddressConst() const { return _registerAddress; }

    protected:
    /// @brief The address of the device.
    uint16_t _deviceAddress;
    /// @brief The address of the register being written or read to.
    uint16_t _registerAddress;
    /// @brief The mode of the I2C.
    I2cConfig::Mode _mode = I2cConfig::Mode::Unknown;
    /// @brief The speed of the I2C.
    I2cConfig::Speed _speed = I2cConfig::Speed::Unknown;
    /// @brief Hardware peripheral number
    I2cConfig::PeripheralNumber _peripheral = I2cConfig::PeripheralNumber::Unknown;
    /// @brief The pin number for the SDA line.
    PinNumber _sda = -1;
    /// @brief Whether the SDA line should have a pull-up resistor.
    bool _sdaPullup = false;
    /// @brief The pin number for the SCL line.
    PinNumber _scl = false;
    /// @brief Whether the SCL line should have a pull-up resistor.
    bool _sclPullup = -1;
};

#endif //__I2C_ABSTRACTION_HPP__