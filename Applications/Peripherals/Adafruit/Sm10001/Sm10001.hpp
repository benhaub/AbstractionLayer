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
//C++
#include <vector>
#include <memory>

namespace Sm10001Drivers {

    /**
     * @enum InputSignalType
     * @brief The input signal that is used to drive the inputs of the H-Bridge.
     */
    enum class InputSignalType {
        Unknown = 0,   ///< Unknown input signal type
        PwmTimer,      ///< Pwm timer input signal
        PwmStandalone, ///< Pwm standalone input signal
        Gpio           ///< Gpio input signal
    };

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

        void setPwms(std::vector<GptmPwmModule> gptPwms) {
            _pwmIsImplementedByGptm = gptPwms.size() == 2;
            _gptPwms.swap(gptPwms);
        }

        void setPwms(std::vector<Pwm> pwms) {
            _pwmIsStandaloneDriver = pwms.size() == 2;
            _pwms.swap(pwms);
        }

        void setGpios(std::vector<Gpio> gpios) {
            _hBridgeIsDrivenByGpio = gpios.size() == 2;
            _gpios.swap(gpios);
        }

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

    struct Drv8872 : public HBridge {
        public:
        Drv8872() = default;
        ~Drv8872() = default;

        //For some reason these PWMs are going up and down in sync. Also If I remove power from the H-Bridge I can still
        //See the signals coming out the other side. That doesn't seem right.
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
     * @param motorInputA The pin number of the motor input A.
     * @param motorInputB The pin number of the motor input B.
     * @post Ownership of the HBridge is transferred to the Sm10001.
     */
    Sm10001(Sm10001Drivers::InputSignalType inputSignalType, std::unique_ptr<Sm10001Drivers::HBridge> &hBridge, PinNumber motorInputA, PinNumber motorInputB) {
        _inputSignalType = inputSignalType;
        _hBridge = std::move(hBridge);
        _motorInputA = motorInputA;
        _motorInputB = motorInputB;

        //For some reason this library will not be able to link unless these constructors are called from the hpp file.
        switch (_inputSignalType) {
            case Sm10001Drivers::InputSignalType::PwmTimer:
                GptmPwmModule();
                break;
            case Sm10001Drivers::InputSignalType::PwmStandalone:
                Pwm();
                break;
            case Sm10001Drivers::InputSignalType::Gpio:
                Gpio();
                break;
            default:
                assert(false);
        }
    }
    /// @brief Destructor
    ~Sm10001() = default;

    static constexpr Count _MotorInputPins = 2;
    static constexpr Milliseconds _PwmPeriod = 20;
    static constexpr Percent _PwmDutyCycle = 25;

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

    private:
    Sm10001Drivers::InputSignalType _inputSignalType;
    std::unique_ptr<Sm10001Drivers::HBridge> _hBridge;
    /// @brief The pin number of the motor input A.
    PinNumber _motorInputA;
    /// @brief The pin number of the motor input B.
    PinNumber _motorInputB;

    constexpr PinNumber mapPeripheralToPinNumber(PeripheralNumber peripheral, ErrorType &error) {
        error = ErrorType::Success;
        switch (peripheral) {
            case PeripheralNumber::Zero:
                return _motorInputA;
            case PeripheralNumber::One:
                return _motorInputB;
            default:
                error = ErrorType::InvalidParameter;
                return 0;
        }
    }
};

#endif //__SM10001_HPP__