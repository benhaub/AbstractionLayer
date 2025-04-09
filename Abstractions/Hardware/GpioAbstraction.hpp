/***************************************************************************//**
* @author  Ben Haubrich
* @file    GpioAbstraction.hpp
* @details Abstraction for GPIO
* @ingroup Abstractions
*******************************************************************************/
#ifndef __GPIO_ABSTRACTION_HPP__
#define __GPIO_ABSTRACTION_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Types.hpp"
//Standard library
#include <stdint.h>

/**
 * @namespace GpioTypes
 * @brief GPIO types
 */
namespace GpioTypes {

    /**
     * @enum PinDirection
     * @brief Pin direction
    */
    enum class PinDirection : uint8_t {
        DigitalUnknown = 0, ///< Unknown pin direction
        DigitalInput,       ///< Pin is digital input
        DigitalOutput       ///< Pin is digital output
    };

    /**
     * @enum InterruptMode
     * @brief The state of the signal that will trigger an interrupt
    */
    enum class InterruptMode : uint8_t {
        Unknown,             ///< Unknown interrupt mode
        LowLevel,            ///< Logic low triggers an
        HighLevel,           ///< Logic high triggers an interrupt
        RisingEdge,          ///< The rising edge triggers an interrupt
        FallingEdge,         ///< The falling edge triggers an interrupt
        RisingOrFallingEdge, ///< Both rising and falling edge
        Disabled             ///< Interrupts are disabled
    };

    /**
     * @enum LogicLevel
     * @brief Logic level
    */
    enum class LogicLevel : uint8_t {
        Unknown = 0, ///< Unknown logic level
        Low,         ///< Low logic level
        High         ///< High logic level
    };
}

/**
 * @class GpioAbstraction
 * @brief Abstraction layer for GPIO pins.
*/
class GpioAbstraction {

    public:
    ///@brief Constructor
    GpioAbstraction() = default;
    ///@brief Destructor
    virtual ~GpioAbstraction() = default;

    /**
     * @brief Init the gpio after configuring.
     * @pre Call setHardwareConfig first.
     * @returns ErrorType::Success if the GPIO was initialized
     * @returns ErrorType::PrerequisitesNotMet if pinDirection(), interruptMode() or pinNumber() are not valid.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType init() = 0;
    /**
     * @brief Set the hadware configuration for the GPIO pin
     * @param basePeripheralRegister The base peripheral register of the gpio pin. Pointer to the hardware instance of the gpio bank that contains the pin.
     * @param pinNumber The pin number of the gpio pin.
     * @param direction The direction of the pin.
     * @param interruptMode The interrupt mode of the pin.
     * @param pullUpEnable Enable pull up resistor on the pin.
     * @param pullDownEnable Enable pull down resistor on the pin.
     * @returns ErrorType::Success if the pin was configured.
     * @returns ErrorType::Failure if the pin was not configured.
     * @returns ErrorType::Invalid parameter if any of the parameters are invalid.
     * @returns ErrorType::NotImplemented if configuring the gpio pin is not implemented.
    */
    virtual ErrorType setHardwareConfig(const uint32_t *basePeripheralRegister, const PinNumber pinNumber, const GpioTypes::PinDirection direction, const GpioTypes::InterruptMode interruptMode, const bool pullUpEnable, const bool pullDownEnable) = 0;
    /**
     * @brief write to the pin and set it's logic level.
     * @param logicLevel The logic level to write to the pin.
     * @returns ErrorType::Success if the pin was written.
     * @returns ErrorType::Failure if the pin was not written.
     * @returns ErrorType::NotImplemented if writing to the gpio pin is not implemented.
     * @returns ErrorType::NotAvailable if the system does not provide gpio pin writing.
     * @returns ErrorType::InvalidParameter if the logic level is not valid.
     * @post The pin should output a voltage level according to the logic level written.
    */
    virtual ErrorType pinWrite(const GpioTypes::LogicLevel &logicLevel) = 0;
    /**
     * @brief Read the logic level of the pin.
     * @param[out] logicLevel The logic level of the pin.
     * @returns ErrorType::Success if the pin was read.
     * @returns ErrorType::Failure if the pin was not read.
     * @returns ErrorType::NotImplemented if reading from the gpio pin is not implemented.
     * @returns ErrorType::NotAvailable if the system does not provide gpio pin reading.
     * @returns ErrorType::InvalidParameter if the logic level is not valid.
     */
    virtual ErrorType pinRead(GpioTypes::LogicLevel &logicLevel) = 0;

    /**
     * @brief Get the pointer to the base register of the gpio peripheral being used.
    */
    const uint32_t &basePeripheralRegister() const { return *_basePeripheralRegister; }
    /**
     * @brief Get the pin number of this gpio
    */
    PinNumber pinNumber() const { return _pinNumber; }
    /**
     * @brief Get the interrupt mode of the gpio pin.
    */
    GpioTypes::InterruptMode interruptMode() const { return _interruptMode; }
    /**
     * @brief Get the direction of the gpio pin.
    */
    GpioTypes::PinDirection direction() const { return _direction; }
    /**
     * @brief True if a pull-up resistor is enabled on this pin
    */
    bool pullUpEnabled() const { return _pullUpEnable; }
    /**
     * @brief True if a pull-down resistor is enabled on this pin.
    */
    bool pullDownEnabled() const { return _pullDownEnable; }


    protected:
    /// @brief The base peripheral register of the gpio pin.
    uint32_t *_basePeripheralRegister;
    /// @brief The pin number of the gpio pin.
    PinNumber _pinNumber;
    /// @brief The interrupt mode of the gpio pin.
    GpioTypes::InterruptMode _interruptMode = GpioTypes::InterruptMode::Unknown;
    /// @brief The direction of the gpio pin.
    GpioTypes::PinDirection _direction = GpioTypes::PinDirection::DigitalUnknown;
    /// @brief True if a pull-up resistor is enabled on this pin.
    bool _pullUpEnable;
    /// @brief True if a pull-down resistor is enabled on this pin.
    bool _pullDownEnable;
};

#endif //__GPIO_ABSTRACTION_HPP__
