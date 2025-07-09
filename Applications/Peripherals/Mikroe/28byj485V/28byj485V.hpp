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
#include "DarlingtonArrayAbstraction.hpp"

/**
 * @class Mikroe28byj485V
 * @brief API for stepper motor driver by Mikroe
 * @see https://www.mikroe.com/step-motor-5v
 */
class Mikroe28byj485V {

    public:
    /// @brief Number of coils in the stepper motor. There is an input for each coil
    static constexpr Count _NumberOfCoils = 4;
    /**
     * @brief Constructor
     * @param darlingtonArray The darlington array used to drive the stepper motor.
     * @post Ownership of the darlington array is transferred to this object.
     */
    Mikroe28byj485V(std::unique_ptr<DarlingtonArrayAbstraction<_NumberOfCoils>> &darlingtonArray) {
        _darlingtonArray = std::move(darlingtonArray);
    }

    /// @brief Tag for logging
    static constexpr char TAG[] = "28byj485V";

    /**
     * @brief Initialize the stepper motor driver
     * @return ErrorType::Success if the motor was initialized.
     * @return ErrorType::Failure otherwise.
     */
    ErrorType init();
    /**
     * @brief Rotate the stepper motor forward
     * @return ErrorType::Success if the motor was rotated.
     * @return ErrorType::Failure otherwise.
     */
    ErrorType rotateForward();

    private:
    /// @brief The darlington array used to drive the stepper motor.
    std::unique_ptr<DarlingtonArrayAbstraction<_NumberOfCoils>> _darlingtonArray;
};

#endif // __28BYJ48_5V_HPP__