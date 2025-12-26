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
//C++
#include <cstdint>

/**
 * @namespace GpioTypes
 * @brief GPIO types
 */
namespace GpioTypes {

    /**
     * @namespace Interrupts
     * @brief Interrupt flags for GPIO
     */
    namespace Interrupts {
        constexpr InterruptFlags LowLevel = 0x01; ///< The low level triggers an interrupt
        constexpr InterruptFlags HighLevel = 0x02; ///< The high level triggers an interrupt
        constexpr InterruptFlags RisingEdge = 0x04; ///< The rising edge triggers an interrupt
        constexpr InterruptFlags FallingEdge = 0x08; ///< The falling edge triggers an interrupt
        constexpr InterruptFlags RisingOrFallingEdge = 0x0C; ///< Rising or falling edge triggers an interrupt
        constexpr InterruptFlags Disabled = 0x00; ///< Interrupts are disabled
    }

    /**
     * @enum PinDirection
     * @brief Pin direction
    */
    enum class PinDirection : uint8_t {
        DigitalUnknown = 0, ///< Unknown pin direction
        DigitalInput,       ///< Pin is digital input
        DigitalOutput,      ///< Pin is digital output
        AnalogOutput,       ///< Pin is analog output
        AnalogInput         ///< Pin is analog input
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
     * @enum DriveType
     * @brief Additional circuitry that is used to drive the pin
     */
    enum class DriveType : uint8_t {
        Unknown = 0, ///< Unknown drive type
        PushPull,    ///< A CMOS is used to actively push or pull the pin to logic high/low giving fast and sharp transitions.
        OpenDrain,   ///< Can drive the line(s) low by turning on an N-type transistor. Pull-up required to bring the line back to high.
    };

    /**
     * @enum DriveStrength
     * @brief The drive strength of the pin to control how much current can go through pin while still being able to hold a stable voltage.
     */
    enum class DriveStrength : uint8_t {
        Unknown = 0,    ///< Unknown drive strength
        TwoMilliAmps,   ///< 2mA
        FourMilliAmps,  ///< 4mA
        EightMilliAmps, ///< 8mA
        TwelveMilliAmps ///< 12mA
    };

    /**
     * @struct GpioParams
     * @brief Contains the parameters used to configure the GPIO.
     */
    struct GpioParams {
        /**
         * @struct HardwareConfig
         * @brief Contains the hardware configuration for the GPIO.
         */
        struct HardwareConfig {
            PeripheralNumber peripheralNumber; ///< The peripheral number to use for the GPIO.
            PinNumber pinNumber;               ///< The pin number to use for the GPIO.
            GpioTypes::PinDirection direction; ///< The pin direction to use for the GPIO.
            InterruptFlags interruptFlags;     ///< The interrupt flags to use for the GPIO.
            bool pullUpEnable;                 ///< True to enable the pull-up resistor, false otherwise.
            bool pullDownEnable;               ///< True to enable the pull-down resistor, false otherwise.
            DriveType driveType;               ///< The drive type of the pin
            DriveStrength driveStrength;       ///< The drive strength of the pin

            /// @brief Constructor
            HardwareConfig() :
              peripheralNumber(PeripheralNumber::Unknown), pinNumber(-1),
              direction(GpioTypes::PinDirection::DigitalUnknown),
              interruptFlags(Interrupts::Disabled), pullUpEnable(false),
              pullDownEnable(false), driveType(DriveType::Unknown), driveStrength(DriveStrength::Unknown) {}
        } hardwareConfig; ///< The hardware configuration for the GPIO.

        /**
         * @struct InterruptConfig
         * @brief Contains the interrupt configuration for the GPIO.
         */
        struct InterruptConfig {
            InterruptFlags interruptFlags;       ///< The interrupt flags to use for the uart
            InterruptCallback interruptCallback; ///< The interrupt callback to use for the uart

            /// @brief Constructor
            InterruptConfig() : interruptFlags(Interrupts::Disabled), interruptCallback(nullptr) {}
        } interruptConfig; ///< The interrupt configuration for the GPIO.

        /// @brief Constructor
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
     * @details Initialization is idempotent.
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
    virtual ErrorType pinWrite(const GpioTypes::LogicLevel &logicLevel) const = 0;
    /**
     * @brief Read the logic level of the pin.
     * @param[out] logicLevel The logic level of the pin.
     * @returns ErrorType::Success if the pin was read.
     * @returns ErrorType::Failure if the pin was not read.
     * @returns ErrorType::NotImplemented if reading from the gpio pin is not implemented.
     * @returns ErrorType::NotAvailable if the system does not provide gpio pin reading.
     * @returns ErrorType::InvalidParameter if the logic level is not valid.
     */
    virtual ErrorType pinRead(GpioTypes::LogicLevel &logicLevel) const = 0;

    /**
     * @brief Configure the GPIO.
     * @param params The parameters to configure the GPIO with.
     * @returns ErrorType::Success
     */
    virtual ErrorType configure(const GpioTypes::GpioParams &params) {
        _gpioParams = static_cast<const GpioTypes::GpioParams &>(params);
        return ErrorType::Success;
    }

    /// @brief Get the parameters that the GPIO was configured with as a constant reference.
    const GpioTypes::GpioParams &gpioParams() const { return _gpioParams; }

    private:
    GpioTypes::GpioParams _gpioParams;
};

#endif //__GPIO_ABSTRACTION_HPP__