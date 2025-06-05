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
//C++
#include <cassert>

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

    /**
     * @brief Set the hardware configurations
     * @param[in] peripheral The peripheral number for this I2c instance.
     * @param[in] mode The mode of the I2C.
     * @param[in] speed The rate at which the I2C bus clock operates.
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
    virtual ErrorType setHardwareConfig(const PeripheralNumber peripheral, const I2cTypes::Mode mode, const I2cTypes::Speed speed, const PinNumber sda, const bool sdaPullUp, const PinNumber scl, const bool sclPullUp) = 0;
    /**
     * @brief enable or disable interrupts.
     * @param[in] arbitrationLost
     * @param[in] nackDetected The transfer was not completed because the target did not acknowledge
     * @param[in] sclLowTimeout The controller has lost clock synchronization with another controller because the SCL line was held low longer than the configured timeout
     * @sa user manual Pg.10, Sect. 3.1.7
     * @param[in] stopDetect When the controller has finished sending data. The stop bit is sent and this interrupt is activated.
     * @param[in] receiveFifoOverflow Can be enabled for controller or transmit mode to fire when the receive FIFO has reached a threshold.
     * @param[in] transmitFifoOverflow Can be enabled for controller or transmitter mode to fire when the transmit FIFO has reached a threshold.
     */
    virtual ErrorType setInterruptConfig(const bool arbitrationLost, const bool nackDetected, const bool sclLowTimeout, const bool stopDetect, const bool receiveFifoOverflow, const bool transmitFifoOverflow) = 0;
    /**
     * @brief transmit data
     * @sa Fnd::CommunicationProtocol::send
     * @param[in] data The data to transmit
     * @param[in] deviceAddress The address of the device you want to transmit to
     * @param[in] registerAddress The address of the register on the device that you want to write to.
     * @param[in] timeout The time to wait to transmit the data.
    */
    virtual ErrorType txBlocking(const std::string &data, uint8_t deviceAddress, uint8_t registerAddress, const Milliseconds timeout) = 0;
    /**
     * @brief transmit data
     * @sa Fnd::CommunicationProtocol::sendNonBlocking
     * @param[in] data The data to transmit
     * @param[in] deviceAddress The address of the device you want to transmit to
     * @param[in] registerAddress The address of the register on the device that you want to write to.
     * @param[in] callback The function to call when the data has been transmitted.
    */
    virtual ErrorType txNonBlocking(const std::shared_ptr<std::string> data, uint8_t deviceAddress, uint8_t registerAddress, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) = 0;
    /**
     * @brief receive data
     * @sa Fnd::CommunicationProtocol::receive
     * @param[out] buffer The received data.
     * @param[in] deviceAddress The address of the device you want to receive from to
     * @param[in] registerAddress The address of the register on the device that you read from.
     * @param[in] timeout The time to wait to receive to the data.
    */
    virtual ErrorType rxBlocking(std::string &buffer, uint8_t deviceAddress, uint8_t registerAddress, const Milliseconds timeout) = 0;
    /**
     * @brief receive data
     * @sa Fnd::CommunicationProtocol::receiveNonBlocking
     * @param[out] buffer The received data.
     * @param[in] deviceAddress The address of the device you want to receive from to
     * @param[in] registerAddress The address of the register on the device that you read from.
     * @param[in] callback The callback to call when receiving has finished.
    */
    virtual ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, uint8_t deviceAddress, uint8_t registerAddress, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) = 0;

    //If you are using the I2C peripheral through an IcCommunicationProtocol pointer then you will need to cast it to an I2C Abstraction or module first.
    ErrorType txBlocking(const std::string &data, const Milliseconds timeout) override { assert(false); return ErrorType::NotSupported; }
    ErrorType txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override { assert(false); return ErrorType::NotSupported; }
    ErrorType rxBlocking(std::string &buffer, const Milliseconds timeout) override { assert(false); return ErrorType::NotSupported; }
    ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override { assert(false); return ErrorType::NotSupported; }

    /// @brief The mode of the I2C.
    I2cTypes::Mode _mode = I2cTypes::Mode::Unknown;
    /// @brief The speed of the I2C.
    I2cTypes::Speed _speed = I2cTypes::Speed::Unknown;
    /// @brief Hardware peripheral number
    PeripheralNumber _peripheral = PeripheralNumber::Unknown;
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