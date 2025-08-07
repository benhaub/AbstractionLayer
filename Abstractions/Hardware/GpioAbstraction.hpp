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

    namespace Interrupts {
        ///< Logic low triggers an interrupt
        constexpr InterruptFlags LowLevel = 0x01;
        ///< Logic high triggers an interrupt
        constexpr InterruptFlags HighLevel = 0x02;
        ///< The rising edge triggers an interrupt
        constexpr InterruptFlags RisingEdge = 0x04;
        ///< The falling edge triggers an interrupt
        constexpr InterruptFlags FallingEdge = 0x08;
        ///< Both rising and falling edge
        constexpr InterruptFlags RisingOrFallingEdge = 0x0C;
        ///< Interrupts are disabled
        constexpr InterruptFlags Disabled = 0x00;
    }

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
     * @enum LogicLevel
     * @brief Logic level
    */
    enum class LogicLevel : uint8_t {
        Unknown = 0, ///< Unknown logic level
        Low,         ///< Low logic level
        High         ///< High logic level
    };

    /**
     * @struct GpioParams
     * @brief Contains the parameters used to configure the GPIO.
     */
    struct GpioParams {

        struct HardwareConfig {
            PeripheralNumber peripheralNumber;
            PinNumber pinNumber;
            GpioTypes::PinDirection direction;
            InterruptFlags interruptFlags;
            bool pullUpEnable;
            bool pullDownEnable;

            HardwareConfig() : peripheralNumber(PeripheralNumber::Unknown), pinNumber(-1), direction(GpioTypes::PinDirection::DigitalUnknown), interruptFlags(Interrupts::Disabled), pullUpEnable(false), pullDownEnable(false) {}
        } hardwareConfig;

        /**
         * @struct InterruptConfig
         * @brief Contains the interrupt configuration for the GPIO.
         */
        struct InterruptConfig {
            InterruptFlags interruptFlags;       ///< The interrupt flags to use for the uart
            InterruptCallback interruptCallback; ///< The interrupt callback to use for the uart

            InterruptConfig() : interruptFlags(Interrupts::Disabled), interruptCallback(nullptr) {}
        } interruptConfig;

        GpioParams() : hardwareConfig(), interruptConfig() {}
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

    /// @brief Tag for logging.
    static constexpr char TAG[] = "Gpio";

    /**
     * @brief Init the gpio after configuring.
     * @pre Call setHardwareConfig first.
     * @returns ErrorType::Success if the GPIO was initialized
     * @returns ErrorType::PrerequisitesNotMet if pinDirection(), interruptMode() or pinNumber() are not valid.
     * @returns ErrorType::Failure otherwise.
     */
    virtual ErrorType init() = 0;
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

    virtual ErrorType configure(const GpioTypes::GpioParams &params) {
        _gpioParams = static_cast<const GpioTypes::GpioParams &>(params);
        return ErrorType::Success;
    }

    const GpioTypes::GpioParams &gpioParams() const { return _gpioParams; }

    private:
    GpioTypes::GpioParams _gpioParams;
};

#endif //__GPIO_ABSTRACTION_HPP__