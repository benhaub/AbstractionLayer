/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   Sm10001.hpp
* @details \b Synopsis: \n Driver for the slide potentiometer by adafruit
* @see https://www.adafruit.com/product/5466
* @ingroup Applications
*******************************************************************************/
#ifndef __SM10001_HPP__
#define __SM10001_HPP__

//AbstractionLayer
#include "GptmPwmModule.hpp"
#include "PwmModule.hpp"
#include "GpioModule.hpp"
#include "AdcModule.hpp"
//C++
#include <vector>
#include <memory>

/**
 * @namespace Sm10001Drivers
 * @brief IC Drivers for the motorized slide potentiometer by adafruit
 */
namespace Sm10001Drivers {
    /**
     * @struct HBridge
     * @brief The H-Bridge that drives the motor.
     */
    struct HBridge {
        public:
        /// @brief Constructor
        HBridge() = default;
        /// @brief Destructor
        virtual ~HBridge() = default;

        /**
         * @brief Drives the motor forward.
         * @returns ErrorType::Success if the motor was driven forward
         * @returns ErrorType::Failure otherwise
         */
        virtual ErrorType driveForward() = 0;
        /**
         * @brief Drives the motor backward.
         * @returns ErrorType::Success if the motor was driven backward
         * @returns ErrorType::Failure otherwise
        */
        virtual ErrorType driveBackward() = 0;
        /**
         * @brief The H-Bridge is put into coast mode for fast current decay
         * @returns ErrorType::Success if the coast was successful
         * @returns ErrorType::Failure otherwise
         */
        virtual ErrorType coast() = 0;
        /**
         * @brief The H-Bridge is put into brake mode for slow current decay
         * @returns ErrorType::Success if the brake was successful
         * @returns ErrorType::Failure otherwise
         */
        virtual ErrorType brake() = 0;

        /**
         * @brief Set the PWMs.
         * @param gptPwms The PWMs implemented by the general purpose timer.
         * @returns ErrorType::Success if the PWMs were set
         * @returns ErrorType::Failure otherwise
         */
        void setPwms(std::vector<GptmPwmModule> &gptPwms) {
            assert(gptPwms.size() == 2);
            _pwmIsImplementedByGptm = true;
            _gptPwms.swap(gptPwms);
        }

        /**
         * @brief Set the PWMs.
         * @param pwms The PWMs implemented by a standalone driver.
         * @returns ErrorType::Success if the PWMs were set
         * @returns ErrorType::Failure otherwise
         */
        void setPwms(std::vector<Pwm> &pwms) {
            assert(pwms.size() == 2);
            _pwmIsStandaloneDriver = true;
            _pwms.swap(pwms);
        }

        /**
         * @brief Set the GPIOs.
         * @param gpios The GPIOs used to drive the H-Bridge.
         * @returns ErrorType::Success if the GPIOs were set
         * @returns ErrorType::Failure otherwise
         */
        void setGpios(std::vector<Gpio> &gpios) {
            assert(gpios.size() == 2);
            _hBridgeIsDrivenByGpio = true;
            _gpios.swap(gpios);
        }

        /// @brief Get a constant reference to the pwms.
        const std::vector<GptmPwmModule> &gptPwmsConst() const { return _gptPwms; }
        /// @brief Get a mutable reference to the pwms
        std::vector<GptmPwmModule> &gptPwms() { return _gptPwms; }
        /// @brief Get a constant reference to the pwms.
        const std::vector<Pwm> &standalonePwmsConst() const { return _pwms; }
        /// @brief Get a mutable reference to the pwms
        std::vector<Pwm> &standalonePwms() { return _pwms; }
        /// @brief Get a constant reference to the gpios.
        const std::vector<Gpio> &gpiosConst() const { return _gpios; }
        /// @brief Get a mutable reference to the gpios
        std::vector<Gpio> &gpios() { return _gpios; }

        protected:
        /// @brief The PWMs implemented by the general purpose timer.
        std::vector<GptmPwmModule> _gptPwms;
        /// @brief The PWMs implemented by a standalone driver.
        std::vector<Pwm> _pwms;
        /// @brief The GPIOs that are used to drive the H-Bridge.
        std::vector<Gpio> _gpios;
        /// @brief True for systems that implement PWM by the general purpose timer.
        bool _pwmIsImplementedByGptm = false;
        /// @brief True for systems that implement PWM by a standalone driver.
        bool _pwmIsStandaloneDriver = false;
        /// @brief True for systems that implement H-Bridge by GPIOs.
        bool _hBridgeIsDrivenByGpio = false;
    };

    /**
     * @struct Drv8872
     * @brief The HBdridge by Texas Instruments
     * @sa https://www.ti.com/lit/ds/symlink/drv8872-q1.pdf?ts=1740601343172&ref_url=https%253A%252F%252Fwww.mouser.it%252F
     */
    struct Drv8872 : public HBridge {
        public:
        /// @brief  Constructor
        Drv8872() = default;
        /// @brief Destructor
        ~Drv8872() = default;

        ErrorType driveForward() override {
            ErrorType error = ErrorType::Failure;
            assert(_pwmIsImplementedByGptm || _pwmIsStandaloneDriver);

            if (_pwmIsImplementedByGptm) {
                error = _gptPwms[0].start();
                if (ErrorType::Success == error) {
                    error = _gptPwms[1].stop();
                }
            }
            else if (_pwmIsStandaloneDriver) {
                error = _pwms[0].start();
                if (ErrorType::Success == error) {
                    error = _pwms[1].stop();
                }
            }
            else if (_hBridgeIsDrivenByGpio) {
                error = _gpios[0].pinWrite(GpioTypes::LogicLevel::High);
                if (ErrorType::Success == error) {
                error = _gpios[1].pinWrite(GpioTypes::LogicLevel::Low);
                }
            }
            else {
                error = ErrorType::NotSupported;
            }

            return error;
        }

        ErrorType driveBackward() override {
            ErrorType error = ErrorType::Failure;
            assert(_pwmIsImplementedByGptm || _pwmIsStandaloneDriver);

            if (_pwmIsImplementedByGptm) {
                error = _gptPwms[0].stop();
                if (ErrorType::Success == error) {
                    error = _gptPwms[1].start();
                }
            }
            else if (_pwmIsStandaloneDriver) {
                error = _pwms[0].stop();
                if (ErrorType::Success == error) {
                    error = _pwms[1].start();
                }
            }
            else if (_hBridgeIsDrivenByGpio) {
                error = _gpios[0].pinWrite(GpioTypes::LogicLevel::Low);
                if (ErrorType::Success == error) {
                    error = _gpios[1].pinWrite(GpioTypes::LogicLevel::High);
                }
            }
            else {
                error = ErrorType::NotSupported;
            }

            return error;
        }

        ErrorType coast() override {
            ErrorType error = ErrorType::Failure;
            assert(_pwmIsImplementedByGptm || _pwmIsStandaloneDriver);

            if (_pwmIsImplementedByGptm) {
                error = _gptPwms[0].stop();
                if (ErrorType::Success == error) {
                    error = _gptPwms[1].stop();
                }
            }
            else if (_pwmIsStandaloneDriver) {
                error = _pwms[0].stop();
                if (ErrorType::Success == error) {
                    error = _pwms[1].stop();
                }
            }
            else if (_hBridgeIsDrivenByGpio) {
                error = _gpios[0].pinWrite(GpioTypes::LogicLevel::Low);
                if (ErrorType::Success == error) {
                    error = _gpios[1].pinWrite(GpioTypes::LogicLevel::Low);
                }
            }
            else {
                error = ErrorType::NotSupported;
            }

            return error;
        }

        ErrorType brake() override {
            ErrorType error = ErrorType::Failure;
            assert(_pwmIsImplementedByGptm || _pwmIsStandaloneDriver);

            if (_pwmIsImplementedByGptm) {
                error = _gptPwms[0].start();
                if (ErrorType::Success == error) {
                    error = _gptPwms[1].start();
                }
            }
            else if (_pwmIsStandaloneDriver) {
                error = _pwms[0].start();
                if (ErrorType::Success == error) {
                    error = _pwms[1].start();
                }
            }
            else if (_hBridgeIsDrivenByGpio) {
                error = _gpios[0].pinWrite(GpioTypes::LogicLevel::High);
                if (ErrorType::Success == error) {
                    error = _gpios[1].pinWrite(GpioTypes::LogicLevel::High);
                }
            }
            else {
                error = ErrorType::NotSupported;
            }

            return error;
        }
    };
}

/**
 * @class Sm10001
 * @brief Driver for the slide potentiometer by adafruit
 */
class Sm10001 {

    public:
    /**
     * @brief Constructor
     * @param hBridge The HBridge to use
     * @param adc The ADC to use
     * @param motorInputA The pin number of the motor input A.
     * @param motorInputB The pin number of the motor input B.
     * @post Ownership of the HBridge is transferred to the SM10001.
     * @post Ownership of the ADC is taken by this SM10001
     */
    Sm10001(std::unique_ptr<Sm10001Drivers::HBridge> &hBridge,
            std::unique_ptr<Adc> &adc,
            PinNumber motorInputA, PinNumber motorInputB) {
        _hBridge = std::move(hBridge);
        _adc = std::move(adc);
        _motorInputA = motorInputA;
        _motorInputB = motorInputB;

        //Note that this only begins to calculate once the period falls below 1000.
        static_assert(1000 / _PwmPeriod <= 250, "Frequency is beyond max reccommended in data sheet.");
    }
    /// @brief Destructor
    ~Sm10001() = default;

    /// @brief Number of input pins on the motor.
    static constexpr Count _MotorInputPins = 2;
    /// @brief Period of the PWMs for the motor.
    static constexpr Microseconds _PwmPeriod = 1000;
    /// @brief Duty cycle of the PWMs for the motor.
    static constexpr Percent _PwmDutyCycle = 100;

    /**
     * @brief init the PWMs for the motor
     * @returns ErrorType::Success if the PWMs were initialized
     * @returns ErrorType::Failure otherwise
     */
    ErrorType init();
    /**
     * @brief Slide forward
     * @returns ErrorType::Success if the slide was successful
     * @returns ErrorType::Failure otherwise
     */
    ErrorType slideForward();
    /**
     * @brief Slide backward
     * @returns ErrorType::Success if the slide was successful
     * @returns ErrorType::Failure otherwise
     */
    ErrorType slideBackward();
    /**
     * @brief Get the voltage drop reading from the potentiometer
     * @param volts The voltage drop reading will be stored here
     * @returns ErrorType::Success if the reading was successful
     * @returns ErrorType::Failure otherwise
     * @post volts is invalid only if ErrorType::Failure is returned.
     */
    ErrorType getVoltageDrop(Volts &volts);

    private:
    /// @brief The H-Bridge that drives the motor
    std::unique_ptr<Sm10001Drivers::HBridge> _hBridge;
    /// @brief The ADC that reads the potentiometer voltage drop.
    std::unique_ptr<Adc> _adc;
    /// @brief The pin number of the motor input A.
    PinNumber _motorInputA;
    /// @brief The pin number of the motor input B.
    PinNumber _motorInputB;
};

#endif //__SM10001_HPP__