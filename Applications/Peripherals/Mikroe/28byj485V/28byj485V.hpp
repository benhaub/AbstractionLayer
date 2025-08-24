/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   28byj485V.hpp
* @details \b Synopsis: \n Driver for stepper motor driver by Mikroe
* @see https://www.mikroe.com/step-motor-5v
* @ingroup Applications
*******************************************************************************/
#ifndef __28BYJ48_5V_HPP__
#define __28BYJ48_5V_HPP__

//AbstractionLayer
#include "DarlingtonArrayFactory.hpp"
#include "OperatingSystemModule.hpp"
//C++
#include <array>

/**
 * @namespace Mikroe28byj485VTypes
 * @brief Contains types related to the 28byj485V stepper motor driver.
 */
namespace Mikroe28byj485VTypes {
    /// @brief The number of input pins that the motor has to control the phases.
    constexpr Count InputPins = 4; 
};

/**
 * @class Mikroe28byj485V
 * @tparam _partNumber The part number of the darlington array that the motor will use.
 * @brief API for stepper motor driver by Mikroe
 * @see https://www.mikroe.com/step-motor-5v
 */
template<DarlingtonArrayFactoryTypes::PartNumber _partNumber, DarlingtonArrayTypes::PwmType _pwmType, std::array<PinNumber, Mikroe28byj485VTypes::InputPins> _inputPins, std::array<PeripheralNumber, Mikroe28byj485VTypes::InputPins> _pwmPeripherals>
class Mikroe28byj485V {

    public:
    /// @brief Tag for logging
    static constexpr char TAG[] = "28byj485V";

    /**
     * @brief Initialize the stepper motor driver
     * @return ErrorType::Success if the motor was initialized.
     * @return ErrorType::Failure otherwise.
     */
    ErrorType init() {

        ErrorType error = DarlingtonArrayFactory::Factory(_partNumber, _darlingtonArray);

        if (ErrorType::Success == error) {
            return std::visit([&](auto &darlingtonArray) -> ErrorType {
                
                if(Mikroe28byj485VTypes::InputPins <= darlingtonArray.inputPins()) {
                    DarlingtonArrayTypes::ConfigurationParameters<DarlingtonArrayFactoryTypes::InputPins(_partNumber)> params;
                    params.pwmType = _pwmType;
                    std::copy_n(_inputPins.begin(), _inputPins.size(), params.pinNumbers.begin());
                    std::copy_n(_pwmPeripherals.begin(), _pwmPeripherals.size(), params.pwmPeripherals.begin());
                    //Default values that you can change later.
                    params.pwmDutyCycles.fill(100.0f);
                    params.pwmPeriods.fill(2E5);
                    params.numberOfPinsUsed = Mikroe28byj485VTypes::InputPins;

                    error = darlingtonArray.configure(params);

                    if (ErrorType::Success == error) {
                        error = darlingtonArray.init();
                    }
                }
                else {
                    error = ErrorType::InvalidParameter;
                }

                return error;

            }, *_darlingtonArray);
        }

        return error;
    }
    /**
     * @brief Rotate the stepper motor forward
     * @param phaseDelay The delay between the transition to a new phase
     * @return ErrorType::Success if the motor was rotated.
     * @return ErrorType::Failure otherwise.
     * @see https://components101.com/motors/28byj-48-stepper-motor
     * @see https://www.python-exemplarisch.ch/index_en.php?inhalt_links=navigation_en.inc.php&inhalt_mitte=raspi/en/steppermotors.inc.php
     */
    ErrorType rotateForward(const Milliseconds phaseDelay) {
        constexpr uint8_t orange = 3;
        constexpr uint8_t pink = 2;
        constexpr uint8_t yellow = 1;
        constexpr uint8_t blue = 0;

        std::visit([&](auto &darlingtonArray) {
            for (int i = 0; i < 2048; i++) {
                darlingtonArray.togglePin(pink, true);
                darlingtonArray.togglePin(orange, false);
                darlingtonArray.togglePin(yellow, false);
                darlingtonArray.togglePin(blue, false);
                OperatingSystem::Instance().delay(phaseDelay);
                darlingtonArray.togglePin(pink, false);
                darlingtonArray.togglePin(orange, true);
                darlingtonArray.togglePin(yellow, false);
                darlingtonArray.togglePin(blue, false);
                OperatingSystem::Instance().delay(phaseDelay);
                darlingtonArray.togglePin(pink, false);
                darlingtonArray.togglePin(orange, false);
                darlingtonArray.togglePin(yellow, true);
                darlingtonArray.togglePin(blue, false);
                OperatingSystem::Instance().delay(phaseDelay);
                darlingtonArray.togglePin(pink, false);
                darlingtonArray.togglePin(orange, false);
                darlingtonArray.togglePin(yellow, false);
                darlingtonArray.togglePin(blue, true);
                OperatingSystem::Instance().delay(phaseDelay);
            }
        }, *_darlingtonArray);

        return ErrorType::Success;
    }

    private:
    /// @brief The darlington array used to drive the stepper motor.
    std::optional<DarlingtonArrayFactoryTypes::DarlingtonArrayFactoryVariant> _darlingtonArray = std::nullopt;
};

#endif // __28BYJ48_5V_HPP__